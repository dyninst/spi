#include "SpUtils.h"
#include "SpInjector.h"

using namespace sp;

// Here we go!
int main(int argc, char *argv[]) {

  if (argc != 3 && argc != 6) {
    sp_print("usage1: %s PID LIB_NAME\n", argv[0]);
    sp_print("    example: %s 2345 /tmp/myagent.so\n", argv[0]);
    sp_print("usage2: %s LOCAL_IP LOCAL_PORT REMOTE_IP REMOTE_PORT LIB_NAME\n", argv[0]);
    sp_print("    example: %s 192.127.4.2 2456 192.125.5.2 3490 /tmp/myagent.so\n", argv[0]);
    exit(0);
  }

	// Non-networking mode: We inject to a process w/ known pid
  if (argc == 3) {
		Dyninst::PID pid = atoi(argv[1]);
		const char* lib_name = argv[2];
		sp_print("Injector [pid = %5d]: INJECTING - %s to pid=%d...", getpid(), lib_name, pid);
		sp_debug("========== Injector ==========");
		SpInjector::ptr injector = SpInjector::create(pid);
		system("/usr/sbin/lsof -i TCP > /tmp/lsofdump");
		injector->inject(lib_name);
	}

	// Networking mode: We inject to processes w/ known ip/port
  else {
		// XXX: currently, we only look at the port number
		//      so we don't do error checking for the time being
		const char* lib_name = argv[5];

		PidSet pid_set;
		sp::addr_to_pids(argv[1], argv[2], argv[3], argv[4], pid_set);
		int try_count = 5;
		while (pid_set.size() < 1 && try_count > 0) {
			fprintf(stderr, "Cannot find any process listening to port %s at %s, %d tries remain\n",
							 argv[4], argv[3], try_count);
			sp::addr_to_pids(argv[1], argv[2], argv[3], argv[4], pid_set);
			sleep(1);
			try_count --;
		}

		// Policy: We inject to every process who listens to rem_port
		for (PidSet::iterator pi = pid_set.begin(); pi != pid_set.end(); pi++) {
			Dyninst::PID pid = *pi;
			sp_print("Injector [pid = %d]: INJECTING - %s to port=%s / pid=%d ...", getpid(), lib_name, argv[4], pid);
			sp_debug("========== Injector ==========");
			SpInjector::ptr injector = SpInjector::create(pid);
			injector->inject(lib_name);
		}
	}

  return 0;
}
