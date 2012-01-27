#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_entry("test_entry");
	agent->set_parse_only(true);
  agent->go();

	SpParser::ptr p = agent->parser();
	PatchFunction* main_func = p->findFunction("main");
	SpObject* obj = p->get_object(main_func);
	if (obj) {
		sp_print(obj->name().c_str());
	} else {
		sp_print("FAILED");
	}
}

AGENT_FINI
void DumpOutput() {
}
