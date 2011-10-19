#include "test_case.h"
#include <dlfcn.h>
#include <sys/ucontext.h>
#include <ucontext.h>

typedef void (*foo_t)();

void foo() {
  printf("indirect call from foo\n");
}

void bar() {
}


typedef struct {
  foo_t f;
} dummy;
int main(int argc, char** argv) {
  //  void* h = dlopen("./indcall_agent.so", RTLD_NOW|RTLD_GLOBAL);
  /*
  if (!h) {
    printf("%s\n", dlerror());
  }
  */

  dummy d;
  d.f = foo;
  d.f();

  d.f = bar;
  d.f();

  foo_t f[3] = {foo, bar, foo};
  f[0]();
  f[1]();

  return 0;
}
