#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h>

int main(int argc, char *argv[]) {
  struct rlimit core_limit;
  core_limit.rlim_cur = RLIM_INFINITY;
  core_limit.rlim_max = RLIM_INFINITY;
  if (setrlimit(RLIMIT_CORE, &core_limit) < 0) return -1;
  while(1) {
    sleep(1);
  }
  return 0;
}
