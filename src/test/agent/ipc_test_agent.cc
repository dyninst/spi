#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

//int level = 0;

void test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

	if (IsIpcWrite(pt)) {
		sp_print("Write: %s @ pid=%d w/ addr %lx",
            f->name().c_str(), getpid(), f->addr());
	}
	else if (IsIpcRead(pt)) {
		sp_print("Read: %s @ pid=%d w/ addr %lx",
             f->name().c_str(), getpid(), f->addr());
	}
  /*
  if (!pt->tailcall()) {
    for (int i = 0; i < level; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s @ pid=%d\n", f->name().c_str(), getpid());
    level++;
  }
  */
  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  // level--;
  
	if (IsIpcWrite(pt)) {
		long size = sp::ReturnValue(pt);
		sp_print("Write size: %ld @ pid=%d", size, getpid());
	} else if (IsIpcRead(pt)) {
		long size = sp::ReturnValue(pt);
		sp_print("Read size: %ld @ pid=%d", size, getpid());
	}
}

AGENT_INIT
void MyAgent() {
  // fprintf(stderr, "AGENT LOADED!\n");
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  libs_to_inst.insert("mod_chunked.so");
  agent->SetLibrariesToInstrument(libs_to_inst);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->EnableIpc(true);
  agent->Go();
}

__attribute__((destructor))
void DumpOutput() {
}

