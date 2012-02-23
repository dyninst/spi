#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {

	PatchFunction* f = callee(pt);
  if (!f) return;
	sp_print("%s", f->name().c_str());
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
	agent->EnableParseOnly(true);
  agent->Go();
	SpParser::ptr p = agent->parser();
	PatchFunction* main_func = p->findFunction("main");
	if (main_func) {
		sp_print("OKAY");
	} else {
		sp_print("FAILED");
	}
}

AGENT_FINI
void DumpOutput() {
}
