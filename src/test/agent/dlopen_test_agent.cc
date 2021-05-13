#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int num_objs = 0;
void* test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return NULL;
  if (f->name().compare("dlopen") == 0) {
    AddrSpace* as = pt->GetObject()->addrSpace();
    num_objs = as->objMap().size();
  }
  return NULL;
}

void test_exit(sp::PointHandle* handle) {
  SpFunction* f = handle->GetCallee();
  if (!f) return;
  if (f->name().compare("dlopen") == 0) {
    AddrSpace* as = handle->GetPoint()->GetObject()->addrSpace();
    // This should be 1
    printf("%lu", (unsigned long)as->objMap().size() - num_objs);
  }  
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->EnableHandleDlopen(true);
  StringSet libs_not_to_inst {"linux-vdso.so",
                                "libdl.so",
                                "libresolv.so",
                                "librt.so",
                                "libcrypto.so",
                                "libstdc++.so",
                                "libm.so",
                                "libgomp.so",
                                "libpthread.so",
                                "libc.so",
                                "ld-linux-x86-64.so"};
  agent->SetLibrariesNotToInstrument(libs_not_to_inst);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->Go();
}
