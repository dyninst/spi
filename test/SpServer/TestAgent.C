#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(Point* pt) {
  // sp_print("%s", callee(pt)->name().c_str());
  sp::propel(pt);
}

void test_after(Point* pt) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_before("test_before");
  agent->set_init_after("test_after");
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
}
