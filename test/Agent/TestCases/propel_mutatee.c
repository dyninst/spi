#include "test_case.h"
#include <dlfcn.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>

double recursion() {
return 0.0;
}

void foo() {
  printf("hello");
}
int main(int argc, char** argv) {
  void* h = dlopen("/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/test/Agent/x86_64-unknown-linux2.4/parser_agent.so", RTLD_NOW);
  if (!h) {
    fprintf(stderr, "%s", dlerror());
  }
  recursion();
}
