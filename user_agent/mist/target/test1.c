#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

void foo2() {
  FILE* fp1 = fopen("../files/test1_1", "r");
  fclose(fp1);
}

void foo1() {
  foo2();
}

void foo5() {
  exit(17);
}

void foo4() {
  foo5();
}

void foo3() {
  char* fname = "../files/test1_2";
  int fp2 = open(fname, O_RDONLY);
  close(fp2);
  foo4();
}

// test1: file open, exit
int main(int argc, char *argv[]) {
  foo1();
  foo3();
  return 0;
}
