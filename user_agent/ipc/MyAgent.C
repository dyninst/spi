#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int level = 0;

void test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

	if (IsIpcWrite(pt)) {
		fprintf(stderr, "Write: %s @ pid=%d w/ addr %lx\n",
            f->name().c_str(), getpid(), f->addr());
	}
	else if (IsIpcRead(pt)) {
		fprintf(stderr, "Read: %s @ pid=%d w/ addr %lx\n",
             f->name().c_str(), getpid(), f->addr());
	}

  if (!pt->tailcall()) {
    for (int i = 0; i < level; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s @ pid=%d\n", f->name().c_str(), getpid());
    level++;
  }

  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  level--;
  // fprintf(stderr, "%s @ pid=%d\n", f->name().c_str(), getpid());
  
  /*
	if (IsIpcWrite(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Write size: %ld @ pid=%d\n", size, getpid());
	} else if (IsIpcRead(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Read size: %ld @ pid=%d\n", size, getpid());
	}
*/
}

AGENT_INIT
void MyAgent() {
  fprintf(stderr, "AGENT LOADED!\n");
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

