#include "test_case.h"

void callee1() {
  dprint("MUTATEE: callee1");
}

void callee2() {
  dprint("MUTATEE: callee2");
}

void func() {
  dprint("MUTATEE: in func");
  callee1();
  callee2();
}

int main(int argc, char** argv) {
  dprint("MUTATEE: enter parser mutatee");
  int count = 1;
  while(count < 10) { sleep(1); count++; }
  func();
  return 0;
}
