#include "test_case.h"

void func() {
  dprint("MUTATEE: in func");
}

void run_mutatee() {
  dprint("MUTATEE: enter parser mutatee");
  int count = 1;
  while(count < 10) { sleep(1); count++; }
  func();
}
