#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {

	PatchFunction* f = callee(pt);
  if (!f) return;
	sp_print("%s", f->name().c_str());
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  libs_to_inst.insert("libtest1.so");
  agent->SetLibrariesToInstrument(libs_to_inst);
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
