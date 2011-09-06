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
  //dprint("MUTATEE: enter event mutatee");
  func();
  int count = 0;
  while (count < 10) {
    count++;
    printf("count: %d\n", count);
    callee2();
  }
  func();
}
