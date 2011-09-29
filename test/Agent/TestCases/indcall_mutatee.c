#include "test_case.h"

void foo() {
  dprint("indirect call from foo");
}

void bar() {
  dprint("indirect call from bar");
}

typedef void (*foo_t)();

typedef struct {
  foo_t f;
} dummy;
int main(int argc, char** argv) {

  dummy d;
  d.f = foo;
  d.f();

  d.f = bar;
  d.f();

  return 0;
}
