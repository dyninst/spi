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
	std::string name = p->get_agent_name();
	sp_print("%s", name.c_str());
}

AGENT_FINI
void DumpOutput() {
}
