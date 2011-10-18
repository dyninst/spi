#include "test_case.h"

void foo() {
  //  printf("indirect call from foo\n");
}

void bar() {
  // printf("indirect call from bar\n");
}

typedef void (*foo_t)();

typedef struct {
  foo_t f;
} dummy;
int main(int argc, char** argv) {

  dummy d;
  d.f = foo;
  d.f();
  printf("%lx\n", d.f);

  d.f = bar;
  d.f();

  return 0;
}
