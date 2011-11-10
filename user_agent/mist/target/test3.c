#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  
  pid_t pid = fork();

  switch(pid) {
  case 0:
    // child
    printf("i'm child: %d\n", getpid());
    fork();
    break;
  case -1:
    // error
    break;
  default:
    // parent
    wait();
    printf("i'm parent: %d\n", getpid());
    break;
  }
  return 0;
}
