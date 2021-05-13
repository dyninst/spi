#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void* test_entry(SpPoint* pt) {
	return NULL;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
	agent->EnableParseOnly(true);
  agent->Go();

	SpParser::ptr p = agent->parser();
	std::string name = p->get_agent_name();
	sp_print("%s", name.c_str());
}

AGENT_FINI
void DumpOutput() {
}
