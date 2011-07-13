#include <stdio.h>
#include <dlfcn.h>

__attribute__((constructor))
void hello() {
  printf("*********************** Hello *******************************!\n");
  void* handle = dlopen("/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/test/Agent/x86_64-unknown-linux2.4/event_agent.so", RTLD_NOW|RTLD_GLOBAL);
  if (!handle) {
    fprintf(stderr, "failed to load %s\n", dlerror());
  }
}
