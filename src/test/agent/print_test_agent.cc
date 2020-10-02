#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;
  std::cout << f->name().c_str() << std::endl;

  sp::Propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  // agent->EnableHandleDlopen(true);
  agent->EnableIpc(true);
  StringSet libs_to_inst;
  libs_to_inst.insert("libtest1.so");
  libs_to_inst.insert("libcalc.so");

  agent->SetLibrariesToInstrument(libs_to_inst);
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}