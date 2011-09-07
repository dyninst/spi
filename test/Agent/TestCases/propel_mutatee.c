#include "test_case.h"

void callee1() {
}

void callee2() {
}

void func() {
  callee1();
  callee2();
}

int main(int argc, char** argv) {
  /*
  func();
  int count = 0;
  int count2 = 3;
  static int count3 = 0;
  while (count < 10) {
    count++;
    char a[126];
    count2 *= 4;
    count3++;
    printf("count: %d, count2: %d\n", count, count2);
    printf("count3: %d\n", count3);
    callee2();
  }
  func();
  */
  /*
  int count = 0;
  while (count < 10) {
    count++;
    printf("count: %d\n", count);
    //printf("return value of printf: %d\n", ret);
  }
  char* a = NULL;
  *a = 'a';
  */
  /*
  int a = 1;
  int b = 2;
  int c = 3;
  printf("%d\n", a);
  printf("%d\n", b);
  printf("%d\n", c);
  */
  int i = 0;
  //int a = 0;
  while (i < 10) {
    i++;
    printf("%d\n", i);
    //a+=2;
  }
  //printf("a: %d\n", a);
}
