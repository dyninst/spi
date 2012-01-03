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

using namespace sp;
using namespace std;

// ----------------------------------------------------------------------------- 
// Tcp Server
// -----------------------------------------------------------------------------
#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
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

int tcp_server() {
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

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
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
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

typedef enum {
  POS_CONNECT,
  POS_WRITE 
} Position;

string tcp_client(const char *hostname, SpTcpWorker* tcp_worker, Position pos) {
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
    return "";
  }

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
			if (pos == POS_CONNECT) {
				TcpChannel* channel = (TcpChannel*)tcp_worker->get_channel(sockfd,
                                    SP_WRITE, (void*)p->ai_addr);
				EXPECT_EQ(channel->type, SP_TCP);
				EXPECT_EQ(channel->remote_ip,
                  inet_lnaof(((sockaddr_in*)p->ai_addr)->sin_addr));
				EXPECT_EQ((int)channel->remote_port, atoi(PORT));
			}
      ret = connect(sockfd, p->ai_addr, p->ai_addrlen);
    } while (ret == -1);
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return "";
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
  // printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  // printf("client: received '%s'\n",buf);
  close(sockfd);

  return buf;
}

/*
// ----------------------------------------------------------------------------- 
// IpcMgr Stuffs
// -----------------------------------------------------------------------------
class IpcMgrUdpTest : public testing::Test {
};

TEST_F(IpcMgrUdpTest, is_tcp) {
}

TEST_F(IpcMgrUdpTest, is_ipc) {
}

TEST_F(IpcMgrUdpTest, tcp_worker) {
}

TEST_F(IpcMgrUdpTest, get_worker) {
}

TEST_F(IpcMgrUdpTest, get_channel) {
}
*/
// ----------------------------------------------------------------------------- 
// Connect
// -----------------------------------------------------------------------------
class TcpConnectTest : public testing::Test {
  public:
	TcpConnectTest() : pid_(-1), is_client_(false) {}

  protected:
	pid_t pid_;
	bool is_client_;
	SpTcpWorker tcp_worker_;

	// Fork a server, then act as a client
  virtual void SetUp() {
		pid_ = fork();

		if (pid_ == 0) {
			is_client_ = false;
			tcp_server();
		} // Child -- server

		else if (pid_ > 0) {
			is_client_ = true;
		} // Parent -- client

	}

	// Kill the server
	virtual void TearDown() {
		if (is_client_) {
			kill(pid_, SIGKILL);
			int status;
			wait(&status);
		}
	}
};

TEST_F(TcpConnectTest, get_channel) {
  if (is_client_) {
		string ret = tcp_client("localhost", &tcp_worker_, POS_CONNECT);
		EXPECT_STREQ("Hello, world!", ret.c_str());
	}
}

TEST_F(TcpConnectTest, inject) {
}

TEST_F(TcpConnectTest, set_start_tracing) {
}

/*
// ----------------------------------------------------------------------------- 
// Write
// -----------------------------------------------------------------------------
class TcpWriteTest : public testing::Test {
};

TEST_F(TcpWriteTest, get_channel) {
}

TEST_F(TcpWriteTest, inject) {
}

TEST_F(TcpWriteTest, set_start_tracing) {
}
*/
