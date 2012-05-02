#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  sp_print("%s", f->name().c_str());

  sp::Propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  agent->SetInitEntry("test_entry");
  agent->Go();
}
