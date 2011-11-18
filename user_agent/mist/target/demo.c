#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>

void foo2() {
  FILE* fp1 = fopen("../files/test1_1", "r");
  if (!fp1) return;
  fclose(fp1);
}

void foo1() {
  foo2();
}

void foo5() {
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

  exit(17);
}

void foo4() {
  void* h = mmap(0, 100, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
  foo5();
}

void* f1(void* arg) {
  printf("f1: i'm in %d\n", pthread_self());
  return 0;
}

void foo3() {
  char* fname = "../files/test1_2";
  int fp2 = open(fname, O_RDONLY);
  if (fp2 == -1) return;
  close(fp2);

  if (setuid(7132) == -1) {
    printf("failed to setuid\n");
  }

  if (setresgid(7132, 7132, 7132) == -1) {
    printf("failed to setuid\n");
  }

  pthread_t t1;
  pthread_create(&t1, NULL, f1, NULL);
  pthread_join(t1, NULL);
  foo4();
}

// all-in-one demo:
//  - fopen/open
//  - exit
//  - mmap
//  - uid/gid
//  - fork
//  - pthread_create
int main(int argc, char *argv[]) {
  foo1();
  foo3();
  return 0;
}
