#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
	agent->EnableParseOnly(true);
  agent->Go();

	SpParser::ptr p = agent->parser();
	PatchFunction* main_func = p->findFunction("main");
	SpObject* obj = FUNC_CAST(main_func)->GetObject();
	if (obj) {
		sp_print(obj->name().c_str());
	} else {
		sp_print("FAILED");
	}
}

AGENT_FINI
void DumpOutput() {
}
