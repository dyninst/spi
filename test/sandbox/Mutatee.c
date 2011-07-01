#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h>


static jmp_buf buf;

void segv_handler(int sig) {
  printf("error code: %d\n", errno);
  perror("Error");
  exit(-1);
}

void int3_handler(int sig) {
  //printf("int3 is hit!!!\n");
  //longjmp(buf,1);
}

int main(int argc, char *argv[]) {
  struct rlimit core_limit;
  core_limit.rlim_cur = RLIM_INFINITY;
  core_limit.rlim_max = RLIM_INFINITY;

  if (setrlimit(RLIMIT_CORE, &core_limit) < 0) return -1;

  //signal(SIGTRAP, SIG_IGN);
  //signal(SIGTRAP, int3_handler);
  //sigset_t x;
  //sigemptyset (&x);
  //sigaddset(&x, SIGTRAP);
  //sigprocmask(SIG_SETMASK, &x, NULL);
  //setjmp(buf);
  //__asm__ ("int $0x3");

  printf("process %d\n", getpid());
  while(1) {
  }
  return 0;
}
