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
		sp_print("Write: %s @ pid=%d w/ addr %lx",
            f->name().c_str(), getpid(), f->addr());
	}
	else if (IsIpcRead(pt)) {
		sp_print("Read: %s @ pid=%d w/ addr %lx\n",
             f->name().c_str(), getpid(), f->addr());
	}
  
  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
	// sp_print("exit func: %s @ pid=%d", f->name().c_str(), getpid());

	if (IsIpcWrite(pt)) {
		long size = sp::ReturnValue(pt);
		sp_print("Write size: %lu @ pid=%d\n", size, getpid());
	} else if (IsIpcRead(pt)) {
		long size = sp::ReturnValue(pt);
		sp_print("Read size: %lu @ pid=%d\n", size, getpid());
	}
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
