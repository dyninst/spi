#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void* test_entry(SpPoint* pt) {

  PatchFunction* f = Callee(pt);
  if (!f) return NULL;
	sp_print("%s at %ld", f->name().c_str(), pthread_self());

  sp::Propel(pt);
  return NULL;
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
  
  agent->EnableMultithread(true);
  
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
