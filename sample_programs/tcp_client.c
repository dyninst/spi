/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
  fprintf(stderr, "Start client PID=%d\n", getpid());
  
  int sockfd, numbytes;  
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }

  // int n1 = fork();
  
  //   // Creating second child. First child
  //   // also executes this line and creates
  //   // grandchild.
  // if (n1 > 0) {
  //   fprintf(stderr, "In parent\n");
  //   int n2 = fork();
  //   if (n2 == 0) {
  //     fprintf(stderr, "child n1 exited\n");
  //     exit(0);
  //   }
  // }
  // if (n1 == 0) {
  //   fprintf(stderr, "child n2 exited\n");
  //   exit(0);
  // }
  

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

  // fprintf(stderr, "Get content: ", s);
  freeaddrinfo(servinfo); // all done with this structure

  while ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) != 0 && numbytes != -1) {
    buf[numbytes] = '\0'; 
		fprintf(stderr, "%s",buf);
  }
	// system("/usr/sbin/lsof -i TCP");
  close(sockfd);
  exit(0);
  return 0;
}
