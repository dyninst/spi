#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_entry("test_entry");
  // agent->set_parse_only(true);
  agent->set_directcall_only(true);
  agent->go();
}

AGENT_FINI
void DumpOutput() {
}
