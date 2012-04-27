#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

	// sp_print("entry func: %s @ pid=%d", f->name().c_str(), getpid());

	if (IsIpcWrite(pt)) {
		fprintf(stderr, "Write: %s @ pid=%d w/ addr %lx\n",
            f->name().c_str(), getpid(), f->addr());
	}
  /*
	else if (IsIpcRead(pt)) {
		fprintf(stderr, "Read: %s @ pid=%d w/ addr %lx\n",
            f->name().c_str(), getpid(), f->addr());
	}
  */
  
  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
	// sp_print("exit func: %s @ pid=%d", f->name().c_str(), getpid());

	if (IsIpcWrite(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Write size: %lu @ pid=%d\n", size, getpid());
	}
  /*
	if (IsIpcRead(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Read size: %lu @ pid=%d\n", size, getpid());
	}
  */
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->EnableIpc(true);
  agent->Go();
}

__attribute__((destructor))
void DumpOutput() {
}
