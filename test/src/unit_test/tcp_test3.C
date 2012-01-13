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
#include "SpIpcMgr.h"
#include "SpChannel.h"
#include "SpUtils.h"

using namespace sp;
using namespace std;

// -----------------------------------------------------------------------------
// Tcp Server
// -----------------------------------------------------------------------------
#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold

char send_string[255];

namespace {

  typedef enum {
    POS_CONNECT,
    POS_WRITE,
    POS_ADDR2PID,
		POS_OOB
  } Position;

  void sigchld_handler(int) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
  }

  // Get sockaddr, IPv4 or IPv6:
  void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }

  int tcp_server(Position pos) {
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    sprintf(send_string, "Hello, world!");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
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
        if (send(new_fd, "Hello", 5, 0) == -1) perror("send");
        if (send(new_fd, ", ", 2, 0) == -1) perror("send");
				if (pos == POS_OOB) {
					// set out of band
					TcpChannel channel;
					channel.fd = new_fd;
					SpTcpWorker worker;
					worker.set_start_tracing(1, &channel);
				}
        if (send(new_fd, "world", 5, 0) == -1) perror("send");
        if (send(new_fd, "!", 1, 0) == -1) perror("send");

        close(new_fd);
        exit(0);
      }
      close(new_fd);  // parent doesn't need this
    }

    return 0;
  }

  // -----------------------------------------------------------------------------
  // Tcp client stuffs
  // -----------------------------------------------------------------------------
#define MAXDATASIZE 100 // max number of bytes we can get at once


  TcpChannel* tcp_client(const char *hostname, SpTcpWorker* tcp_worker, Position pos) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return NULL;
    }

    TcpChannel* channel = NULL;
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1) {
        perror("client: socket");
        continue;
      }

      int ret = -1;
      do {
        // Testing starts
        if (pos == POS_CONNECT || pos == POS_OOB) {
          channel = (TcpChannel*)tcp_worker->get_channel(sockfd,
                                                         SP_WRITE, (void*)p->ai_addr);
          EXPECT_TRUE(channel != NULL);
        }

        ret = connect(sockfd, p->ai_addr, p->ai_addrlen);

        if (pos == POS_CONNECT) {

          // Verify remote ip/port
          EXPECT_EQ(channel->remote_ip.s_addr,
                    ((sockaddr_in*)p->ai_addr)->sin_addr.s_addr);
          EXPECT_EQ((int)channel->remote_port,
                    htons(((sockaddr_in*)p->ai_addr)->sin_port));

          // Verify local ip/port
					/*
          sockaddr_in loc_sa;
          memset(&loc_sa, 0, sizeof(sockaddr_in));
          socklen_t loc_len = sizeof(sockaddr_in);
          if (getsockname(sockfd, (sockaddr*)&loc_sa, &loc_len) == -1) {
            perror("getsockname");
          }
          EXPECT_EQ((in_addr_t)channel->local_ip.s_addr, loc_sa.sin_addr.s_addr);
          EXPECT_EQ((unsigned short)channel->local_port, htons(loc_sa.sin_port));
					*/
        }
      } while (ret == -1);
      break;
    }

    if (p == NULL) {
      fprintf(stderr, "client: failed to connect\n");
      return NULL;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    // printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, 5, 0)) != 0 && numbytes != -1) {
			buf[5] = '\0';
			EXPECT_STREQ("Hello", buf);
    }


    if ((numbytes = recv(sockfd, buf, 2, 0)) != 0 && numbytes != -1) {
			buf[2] = '\0';
			EXPECT_STREQ(", ", buf);
    }

		if (pos == POS_OOB) {
			tcp_worker->start_tracing(sockfd);
		}

    if ((numbytes = recv(sockfd, buf, 5, 0)) != 0 && numbytes != -1) {
			buf[5] = '\0';
			EXPECT_STREQ("world", buf);
    }

    if ((numbytes = recv(sockfd, buf, 1, 0)) != 0 && numbytes != -1) {
			buf[1] = '\0';
			EXPECT_STREQ("!", buf);
    }

		if (pos == POS_OOB) {
			char yes_or_no = tcp_worker->start_tracing(sockfd);
			EXPECT_EQ(yes_or_no, 1);
		}

    close(sockfd);

    return channel;
  }

  // -----------------------------------------------------------------------------
  // To test right before connect
  //   - TcpWorker::inject() and  TcpWorker::getChannel
  //
  // Setting:
  // - Server: tcp_server
  // - Client: this executable (tcp_test)
  // -----------------------------------------------------------------------------
  class TcpConnectTest3 : public testing::Test {
  public:
    TcpConnectTest3() {}

  protected:
    pid_t server_pid_;

    // Start a server on remote machine, then fork a client on local machine
    virtual void SetUp() {
			server_pid_ = fork();
			if (server_pid_ == 0) {
				system("ssh wasabi sh /afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/spi/test/x86_64-unknown-linux2.4/tcp_server.sh");
			}
    }

    // Kill the server
    virtual void TearDown() {
			if (server_pid_ > 0) {
				system("ssh wasabi killall tcp_server");
				kill(server_pid_, SIGKILL);
			}
    }
  };

	/*
  TEST_F(TcpConnectTest3, get_channel) {
		const char* hostname = "localhost";
		// const char* hostname = "wasabi";
		TcpChannel* channel = tcp_client(hostname, &tcp_worker_, POS_CONNECT);
		EXPECT_EQ(channel->type, SP_TCP);
  }
*/

  TEST_F(TcpConnectTest3, inst_server_not_inst_client) {
		if (server_pid_ > 0) {
			sleep(10);
			// Inject to server
			TcpChannel c;
			c.remote_ip.s_addr = inet_addr("128.105.166.35");
			c.remote_port = 3490;

			// SpTcpWorker tcp_worker;
			// bool ret = tcp_worker.inject(&c, (char*)"./ipc_test_agent.so", (char*)"./Injector", (char*)"./libijagent.so");
			// EXPECT_TRUE(ret);
			// char cmd[1024];
			// sprintf(cmd, "./Injector %d ./ipc_test_agent.so", server_pid_);
			// system(cmd);

			// Trigger server to output things
      FILE* fp = popen("./tcp_client wasabi", "r");
			char buf[1024];
			while (fgets(buf, 1024, fp) != NULL) {
				fprintf(stderr, "From client: %s\n", buf);
			}
			pclose(fp);

		}
  }

	/*
  TEST_F(TcpConnectTest3, not_inst_server_not_inst_client) {
		if (server_pid_ > 0) {
			sleep(2);
			system("./tcp_client wasabi");
		}
  }
*/
	/*
	// Out-of-band mechanism
  TEST_F(TcpConnectTest3, oob) {
		// Use our own server routine
		kill(pid_, SIGKILL);
		int status;
		wait(&status);
		pclose(server_);
		server_ = NULL;

		pid_ = fork();
		if (pid_ == 0) {
			tcp_server(POS_OOB);
		} // Child as server

		else if (pid_ > 0) {
			const char* hostname = "localhost";
			// const char* hostname = "wasabi";
			TcpChannel* channel = tcp_client(hostname, &tcp_worker_, POS_OOB);
			ASSERT_TRUE(channel);
			EXPECT_EQ(channel->type, SP_TCP);
		} // Parent as client
  }
*/
}
