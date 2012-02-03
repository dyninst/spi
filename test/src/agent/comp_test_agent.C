#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {

	PatchFunction* f = callee(pt);
  if (!f) return;
	// SpObject* obj = static_cast<SpObject*>(pt->block()->obj());
	// sp_print("%s in %s", f->name().c_str(), obj->name().c_str());
	sp_print("%s", f->name().c_str());
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_entry("test_entry");
	if (getenv("SP_DIRECTCALL_ONLY")) {
		agent->set_directcall_only(true);
	}
	if (getenv("SP_TRAP")) {
		agent->set_trap_only(true);
	}
  agent->go();
}

AGENT_FINI
void DumpOutput() {
}
