#include "SpInjector.h"

using sp::SpInjector;

// Here we go!
int main(int argc, char *argv[]) {
  if (argc != 3) {
    sp_print("usage: %s PID LIB_NAME", argv[0]);
    exit(0);
  }
  Dyninst::PID pid = atoi(argv[1]);
  const char* lib_name = argv[2];
  sp_print("Injector [pid = %5d]: INJECTING - %s ...", getpid(), lib_name);
  sp_debug("========== Injector ==========");
  SpInjector::ptr injector = SpInjector::create(pid);
  injector->inject(lib_name);
  return 0;
}
