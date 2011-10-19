#include "test_case.h"
#include <dlfcn.h>
#include <sys/ucontext.h>
#include <ucontext.h>

void foo() {
  printf("indirect call from foo\n");
}

void bar() {
  printf("indirect call from bar\n");
}

typedef void (*foo_t)();

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
  //printf("ccc");
  /*
  ucontext_t c;
  getcontext(&c);
  mcontext_t m = c.uc_mcontext;

  printf("edi: %lx\n", m.gregs[0]);
  printf("esi: %lx\n", m.gregs[1]);
  printf("ebp: %lx\n", m.gregs[2]);
  printf("esp: %lx\n", m.gregs[3]);
  printf("edx: %lx\n", m.gregs[4]);
  printf("ebx: %lx\n", m.gregs[5]);
  printf("ecx: %lx\n", m.gregs[6]);
  printf("eax: %lx\n", m.gregs[7]);
  */
  d.f();

  d.f = bar;
  d.f();


  return 0;
}
