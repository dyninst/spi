#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void* test_entry(PointCallHandle* handle) {
  SpFunction* f = handle->GetCallee();
  SpPoint* pt = handle->GetPoint();
  if (!f) return NULL;
  sp_print("ENTER %s", f->GetMangledName().c_str());
  sp::Propel(pt);
  return NULL;
}

void test_exit(sp::PointCallHandle* handle) {
  if (!handle->GetCallee()) return;
  sp_print("LEAVE %s", handle->GetCallee()->GetMangledName().c_str());
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
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

AGENT_FINI
void DumpOutput() {
}
