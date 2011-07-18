#include "test_case.h"

void func1() {
  dprint("MUTATEE: func1");
}

void func2() {
  dprint("MUTATEE: func2");
}

void run_mutatee() {
  dprint("MUTATEE: enter event mutatee");
  func1();
  func2();
}
