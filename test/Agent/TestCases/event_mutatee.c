#include "test_case.h"

void func1() {
  dprint("MUTATEE: func1");
}

void func2() {
  dprint("MUTATEE: func2");
}

void func() {
  func2();
  func1();
}
void main() {
  dprint("MUTATEE: enter event mutatee");
  func1();
  func2();

  int count = 0;
    while(count < 10) {
      dprint("MUTATEE: mutatee alive");
      sleep(1);
      count++;
  }
  func();
}
