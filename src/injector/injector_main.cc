#include <libgen.h>
#include <stdio.h>
#include <string.h>

#include "common/utils.h"

#include "injector/injector.h"

using namespace sp;

FILE*         g_debug_fp = NULL;

// Here we go!
int main(int argc, char *argv[]) {

  if (argc != 4) {
    sp_print("usage1: %s pid PID LIB_NAME\n", argv[0]);
    sp_print("    example: %s pid 2345 /tmp/myagent.so\n", argv[0]);
    sp_print("usage2: %s port REMOTE_PORT LIB_NAME\n",
             argv[0]);
    sp_print("    example: %s port 8080 "
             "/tmp/myagent.so\n", argv[0]);
    exit(0);
  }

  // Non-networking mode: We inject to a process w/ known pid
  if (strcmp(argv[1], "pid") == 0) {
    // sp_print("Non-network mode");
    Dyninst::PID pid = atoi(argv[2]);
    const char* lib_name = argv[3];
    sp_print("Injector [pid = %5d]: INJECTING - %s to pid=%d...",
             getpid(), lib_name, pid);
    sp_debug("========== Injector ==========");
    SpInjector::ptr injector = SpInjector::Create(pid);
    injector->Inject(lib_name);
  }

  // Networking mode: We inject to processes w/ known ip/port
  else if (strcmp(argv[1], "port") == 0) {
    // sp_print("Network mode");
    
    // XXX: currently, we only look at the port number
    //      so we don't do error checking for the time being
    const char* lib_name = argv[3];
    
    PidSet pid_set;
    sp::GetPidsFromAddrs("1.1.1.1", argv[2], pid_set);
    int try_count = 5;
    while (pid_set.size() < 1 && try_count > 0) {
      sp_debug("Cannot find any process listening to port %s,"
              " %d tries remain\n", argv[2], try_count);
      sp::GetPidsFromAddrs("1.1.1.1", argv[2], pid_set);
      sleep(1);
      try_count --;
    }

    // Policy: We inject to every process who listens to rem_port
    for (PidSet::iterator pi = pid_set.begin(); pi != pid_set.end(); pi++) {
      Dyninst::PID pid = *pi;
      sp_print("Injector [pid = %d]: INJECTING - %s to port=%s / pid=%d ...",
               getpid(), lib_name, argv[2], pid);
      sp_debug("========== Injector ==========");
      SpInjector::ptr injector = SpInjector::Create(pid);
      injector->Inject(lib_name);
    }
  }

  return 0;
}
