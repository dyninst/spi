#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;
  sp_print("ENTER %s", f->GetMangledName().c_str());
  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  sp_print("LEAVE %s", f->GetMangledName().c_str());
  if (!f) return;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  libs_to_inst.insert("libtest1.so");
  agent->SetLibrariesToInstrument(libs_to_inst);
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
