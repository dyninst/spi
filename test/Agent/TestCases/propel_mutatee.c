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

  int i = 0;
  //int a = 0;
  int j = 0;
  for (; j < 5; j++) {
  printf("outer %d\n", j);
  while (i < 4) {
    i++;
    printf("  inner %d\n", i);
    func();
    callee1();
  }
  func();
  i = 0;
  }
  //printf("a: %d\n", a);
}
