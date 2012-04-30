// Google test/mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Headers for TCP Client/Server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// Self-propelled stuffs
#include "SpInc.h"
#include "agent/ipc/ipc_mgr.h"
#include "agent/ipc/ipc_workers/tcp_worker_impl.h"

using namespace sp;
using namespace std;

namespace {

#define PORT "3490"
#define MAXDATASIZE 100
#define BACKLOG 10

	typedef enum {
		INJECT,
		GET_CHANNEL,
		OOB
	} TestCmd;

// ----------------------------------------------------------------------------- 
// Server
// -----------------------------------------------------------------------------

	void sigchld_handler(int) {
		while(waitpid(-1, NULL, WNOHANG) > 0);
	}

	// get sockaddr, IPv4 or IPv6:
	void *get_in_addr(struct sockaddr *sa) {
		if (sa->sa_family == AF_INET) {
			return &(((struct sockaddr_in*)sa)->sin_addr);
		}
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}



	int tcp_server(int family, TestCmd cmd = OOB) {
		int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
		struct addrinfo hints, *servinfo, *p;
		struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		int yes=1;
		char s[INET6_ADDRSTRLEN];
		int rv;

		memset(&hints, 0, sizeof hints);
		hints.ai_family = family;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE; // use my IP

		if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
													 p->ai_protocol)) == -1) {
				perror("server: socket");
				continue;
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
										 sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1);
			}

			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("server: bind");
				continue;
			}

			break;
		}

		if (p == NULL)  {
			fprintf(stderr, "server: failed to bind\n");
			return 2;
		}

		freeaddrinfo(servinfo); // all done with this structure

		if (listen(sockfd, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}

		sa.sa_handler = sigchld_handler; // reap all dead processes
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
		}

		// printf("server: waiting for connections...\n");

		while(1) {  // main accept() loop
			//do {
			sin_size = sizeof their_addr;
			new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("accept");
				continue;
			}

			inet_ntop(their_addr.ss_family,
								get_in_addr((struct sockaddr *)&their_addr),
								s, sizeof s);
			// printf("server: got connection from %s\n", s);

			if (!fork()) { // this is the child process
				close(sockfd); // child doesn't need the listener

				SpTcpWorker tcp_worker;
				TcpChannel* channel = (TcpChannel*)tcp_worker.get_channel(new_fd,
																																	SP_WRITE, NULL);
				EXPECT_TRUE(channel != NULL);
			
				if (send(new_fd, "Hello", 5, 0) == -1)
					perror("send");
				if (send(new_fd, ", ", 2, 0) == -1)
					perror("send");

				if (cmd == OOB) {
					// set out of band
					channel->fd = new_fd;
					SpTcpWorker worker;
					worker.set_start_tracing(1, channel);
				}

				if (send(new_fd, "world", 5, 0) == -1)
					perror("send");
				if (send(new_fd, "!", 1, 0) == -1)
					perror("send");

				close(new_fd);
				exit(0);
			}
			close(new_fd);  // parent doesn't need this
		}

		return 0;
	}


	// ----------------------------------------------------------------------------- 
	// Client
	// -----------------------------------------------------------------------------

	std::string tcp_client(const char *hostname, TestCmd cmd = GET_CHANNEL) {
		int sockfd, numbytes;  
		char buf[MAXDATASIZE];
		struct addrinfo hints, *servinfo, *p;
		int rv;
		char s[INET6_ADDRSTRLEN];

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return "";
		}

		TcpChannel* channel;
		SpTcpWorker tcp_worker;
		// loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
													 p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}
      int ret = -1;
      do {
        // Create channel
				channel = (TcpChannel*)tcp_worker.get_channel(sockfd,
																											SP_WRITE, (void*)p->ai_addr);

        ret = connect(sockfd, p->ai_addr, p->ai_addrlen);

				// Verify
      } while (ret == -1);
			EXPECT_TRUE(channel != NULL);
			break;
		}

		if (cmd == INJECT) {
			SpTcpWorker tcp_worker;
			std::string agent_path = getenv("SP_AGENT_DIR");
			agent_path += "/";
			agent_path += "inject_test_agent.so";
			bool ret = tcp_worker.inject(channel, (char*)agent_path.c_str());
			EXPECT_TRUE(ret);
		}

		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return "";
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
							s, sizeof s);

		freeaddrinfo(servinfo); // all done with this structure

		std::string out;
		while ((numbytes = recv(sockfd, buf, 1, 0)) != 0 && numbytes != -1) {
			if (cmd == OOB) {
				if (tcp_worker.start_tracing(sockfd)) out += buf[0];
			} 
			else {
				out += buf[0];
			}
		}
		close(sockfd);
		return out;
	}

  // -----------------------------------------------------------------------------
  // To test right before connect
  // -----------------------------------------------------------------------------
  class TcpTest1 : public testing::Test {
  public:
    TcpTest1()  {
      mutatee_prefix_ = " LD_LIBRARY_PATH=test_mutatee::tmp/lib:$LD_LIBRARY_PATH ";
      preload_prefix_ = " LD_PRELOAD=$SP_DIR/$PLATFORM/test_agent/ipc_test_agent.so ";
    }

  protected:
    string mutatee_prefix_;
    string preload_prefix_;
    
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
  };

  TEST_F(TcpTest1, get_channel_ipv6) {
		pid_t pid = fork();
		if (pid == 0) {
			tcp_server(AF_INET6);
		} else if (pid > 0) {
			const char* hostname = "localhost";
			tcp_client(hostname);
			kill(pid, SIGKILL);
			int status;
			wait(&status);
		}
  }


  TEST_F(TcpTest1, get_channel_ipv4) {
		pid_t pid = fork();
		if (pid == 0) {
			tcp_server(AF_INET);
		} else if (pid > 0) {
			const char* hostname = "localhost";
			tcp_client(hostname);
			kill(pid, SIGKILL);
			int status;
			wait(&status);
		}
  }

  TEST_F(TcpTest1, inject) {
    string cmd = mutatee_prefix_ + "test_mutatee/tcp_server6.exe";
    
		FILE* fp = popen(cmd.c_str(), "r");
		const char* hostname = "localhost";
		tcp_client(hostname, INJECT);
		// system("./tcp_client localhost");
		tcp_client(hostname);
		char buf[256];
		EXPECT_TRUE(fgets(buf, 256, fp) != NULL);
		EXPECT_TRUE(fgets(buf, 256, fp) != NULL);
		EXPECT_TRUE(fgets(buf, 256, fp) != NULL);
		EXPECT_TRUE(fgets(buf, 256, fp) != NULL);
		EXPECT_STREQ(buf, "AGINJECTED\n");
		system("killall tcp_server6.exe");
  }

	// Out-of-band mechanism
  TEST_F(TcpTest1, oob) {
		pid_t pid = fork();
		if (pid == 0) {
			tcp_server(OOB);
		} // Child as server

		else if (pid > 0) {
			const char* hostname = "localhost";
			// const char* hostname = "wasabi";
			tcp_client(hostname, OOB);
		} // Parent as client

		kill(pid, SIGKILL);
		int status;
		wait(&status);
  }

#if 0
  TEST_F(TcpTest1, inject_remote) {
		pid_t pid = fork();
		if (pid == 0) {
			system("ssh localhost tcp_server6.exe");
			sleep(5);
		} // Child as server

		else if (pid > 0) {
			const char* hostname = "wasabi";
			tcp_client(hostname, INJECT);
			system("ssh wasabi killall tcp_server6.exe");
			kill(pid, SIGKILL);
			int status;
			wait(&status);
		}
  }
#endif
}
