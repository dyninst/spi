#include "SpInc.h"
#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

mist::Mist g_mist;

void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  sp::Propel(pt);
}


AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  agent->SetInitEntry("test_entry");
  agent->Go();
}
