#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int num_objs = 0;
void test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  if (f->name().compare("dlopen") == 0) {
    AddrSpace* as = pt->GetObject()->addrSpace();
    num_objs = as->objMap().size();
  }
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  if (f->name().compare("dlopen") == 0) {
    AddrSpace* as = pt->GetObject()->addrSpace();
    // This should be 1
    printf("%lu", as->objMap().size() - num_objs);
  }  
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->EnableHandleDlopen(true);
  StringSet libs_to_inst;
  libs_to_inst.insert("libz");
  agent->SetLibrariesToInstrument(libs_to_inst);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->Go();
}
