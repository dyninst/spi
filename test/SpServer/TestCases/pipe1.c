/* Preload agent.so before fork() */

#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>

#define STR "hello!pipe\n"

void foo(int* pfd, char* buf) {
  while (read(pfd[0], buf, 1) > 0)
    write(STDOUT_FILENO, buf, 1);
}

int main(int argc, char *argv[]) {
  int pfd[2];
  pid_t cpid;
  char buf;

  printf("parent: %d\n", getpid());
  if (pipe(pfd) == -1) { perror("pipe"); exit(EXIT_FAILURE); }

  cpid = fork();
  if (cpid == -1) { perror("fork"); exit(EXIT_FAILURE); }

  if (cpid == 0) {    /* Child reads from pipe */
    close(pfd[1]);          /* Close unused write end */

    foo(pfd, &buf);
    write(STDOUT_FILENO, "\n", 1);

    close(pfd[0]);
    _exit(EXIT_SUCCESS);

  } else {            /* Parent writes argv[1] to pipe */
    close(pfd[0]);          /* Close unused read end */

    /* Manually load the agent for testing */
    /*    void* h = dlopen("./TestAgent.so", RTLD_NOW|RTLD_GLOBAL);
    if (!h) {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
    }
    */
    int i;
    for (i = 0; i < 3; i++)
    write(pfd[1], STR, strlen(STR));
    close(pfd[1]);          /* Reader will see EOF */
    wait(NULL);             /* Wait for child */
    exit(EXIT_SUCCESS);
  }
  
  return 0;
}
