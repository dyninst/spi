#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void* test_entry(SpPoint* pt) {
  PatchFunction* f = Callee(pt);
  if (!f) return NULL;
  sp_print("%s", f->name().c_str());
  sp::Propel(pt);
  return NULL;
}

AGENT_INIT
void MyAgent() {
  StringSet preinst_funcs;
  preinst_funcs.insert("main");

  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  SpAgent::ptr agent = SpAgent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {}
