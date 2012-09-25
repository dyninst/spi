/*
  It will output trace data to /tmp. Please read README file for more details.
 */

#include "SpInc.h"
#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

// mist::Mist g_mist;

void mist_entry(SpPoint* pt) {

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
  
  // g_mist.run(pt, f);
  sp::Propel(pt);
}

void mist_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

	if (IsIpcWrite(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Write size: %lu @ pid=%d\n", size, getpid());
	} else if (IsIpcRead(pt)) {
		long size = sp::ReturnValue(pt);
		fprintf(stderr, "Read size: %lu @ pid=%d\n", size, getpid());
	}
  
  // g_mist.post_run(pt, f);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  // StringSet libs_to_inst;
  // agent->SetLibrariesToInstrument(libs_to_inst);
  agent->SetInitEntry("mist_entry");
  agent->SetInitExit("mist_exit");
  agent->EnableIpc(true);
  agent->Go();
}
