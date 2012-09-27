#include <libgen.h>
#include <stdio.h>

#include "common/utils.h"

#include "injector/injector.h"

using namespace sp;

FILE*         g_debug_fp = NULL;

// Here we go!
int main(int argc, char *argv[]) {

  if (argc != 3 && argc != 4) {
    sp_print("usage1: %s PID LIB_NAME\n", argv[0]);
    sp_print("    example: %s 2345 /tmp/myagent.so\n", argv[0]);
    sp_print("usage2: %s REMOTE_IP REMOTE_PORT LIB_NAME\n",
             argv[0]);
    sp_print("    example: %s 192.125.5.2 3490 "
             "/tmp/myagent.so\n", argv[0]);
    exit(0);
  }

  // Non-networking mode: We inject to a process w/ known pid
  if (argc == 3) {
    // sp_print("Non-network mode");
    Dyninst::PID pid = atoi(argv[1]);
    const char* lib_name = argv[2];
    sp_print("Injector [pid = %5d]: INJECTING - %s to pid=%d...",
             getpid(), lib_name, pid);
    sp_debug("========== Injector ==========");
    SpInjector::ptr injector = SpInjector::Create(pid);
    /*
    std::string lsof_path;
    if (getenv("SP_LSOF") != NULL)
      lsof_path = getenv("SP_LSOF");
    else
      lsof_path = "lsof";
    char cmd[1024];
    snprintf(cmd, 1024, "%s -i TCP > /tmp/lsofdump", lsof_path.c_str());
    system(cmd);
    */
    injector->Inject(lib_name);
  }

  // Networking mode: We inject to processes w/ known ip/port
  else {
    // sp_print("Network mode");
    
    // XXX: currently, we only look at the port number
    //      so we don't do error checking for the time being
    const char* lib_name = argv[3];
    
    PidSet pid_set;
    sp::GetPidsFromAddrs(argv[1], argv[2], pid_set);
    int try_count = 5;
    while (pid_set.size() < 1 && try_count > 0) {
      sp_debug("Cannot find any process listening to port %s at %s,"
              " %d tries remain\n", argv[2], argv[1], try_count);
      sp::GetPidsFromAddrs(argv[1], argv[2], pid_set);
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
