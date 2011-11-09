#include <stdio.h>
#include <fcntl.h>

void foo2() {
  FILE* fp1 = fopen("../files/test1_1", "r");
  fclose(fp1);
}

void foo1() {
  foo2();
}

void foo3() {
  char* fname = "../files/test1_2";
  int fp2 = open(fname, O_RDONLY);
  close(fp2);
}

// test1: file open
int main(int argc, char *argv[]) {
  foo1();
  foo3();
  return 0;
}
