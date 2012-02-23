#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {

  PatchFunction* f = callee(pt);
  if (!f) return;

	sp_print("entry func: %s @ pid=%d", f->name().c_str(), getpid());
	/*
	if (is_ipc_write(pt)) {
		fprintf(stderr, "Write: %s @ pid=%d w/ addr %lx\n", f->name().c_str(), getpid(), f->addr());
	}
	else if (is_ipc_read(pt)) {
		fprintf(stderr, "Read: %s @ pid=%d w/ addr %lx\n", f->name().c_str(), getpid(), f->addr());
	}
	*/
  sp::propel(pt);
}

void test_exit(SpPoint* pt) {
  PatchFunction* f = callee(pt);
  if (!f) return;
	/*
	//sp_print("exit func: %s @ pid=%d", f->name().c_str(), getpid());

	if (is_ipc_write(pt)) {
		long size = sp::retval(pt);
		fprintf(stderr, "Write size: %lu @ pid=%d\n", size, getpid());
	}
	else if (is_ipc_read(pt)) {
		long size = sp::retval(pt);
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
