#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(SpPoint* pt) {

  PatchFunction* f = callee(pt);
  if (!f) return;
	sp_print("%s", f->name().c_str());
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_before("test_before");
  agent->go();
}

AGENT_FINI
void DumpOutput() {
}
