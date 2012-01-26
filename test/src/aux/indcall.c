#include <dlfcn.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <stdio.h>

typedef void (*foo_t)();

void foo() {
	sleep(1);
}

void bar() {
}

typedef struct {
  foo_t f;
} dummy;

int main(int argc, char** argv) {
  dummy d;
  d.f = foo;
  d.f();

  d.f = bar;
  d.f();

  bar();

  return 0;
}
