#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

FILE* fp = NULL;
FILE* g_output_fp;
pid_t previous_pid = 0;

int level = -1;
typedef map<int, Address> LevelMap;
LevelMap level_map;

void* test_entry(SpPoint* pt) {

  level ++;
  if (pt->tailcall()) level--;
  
  SpFunction* f = Callee(pt);
  if (!f) return NULL;

  sp_print("%s", f->name().c_str());

  sp::Propel(pt);
  return NULL;
}

void spi_test_exit(PointHandle* handle) {
  level --;
  if (handle->GetCallee()) {
    sp_print("Exiting %s", handle->GetCallee()->name().c_str());
  }
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

  StringSet funcs_not_to_inst;
  funcs_not_to_inst.insert("_Znwm");
  funcs_not_to_inst.insert("_Znam");
  funcs_not_to_inst.insert("_ZdlPv");
  funcs_not_to_inst.insert("_ZdaPv");
  funcs_not_to_inst.insert("malloc");
  funcs_not_to_inst.insert("free");
  funcs_not_to_inst.insert("strdup");
  funcs_not_to_inst.insert("get_mySubSystem");
  funcs_not_to_inst.insert("strcasecmp");
  funcs_not_to_inst.insert("strlen");
  funcs_not_to_inst.insert("tolower");
  funcs_not_to_inst.insert("__wrap_exit");
  funcs_not_to_inst.insert("realloc");
  funcs_not_to_inst.insert("memcpy");
  funcs_not_to_inst.insert("memcmp");
  funcs_not_to_inst.insert("memset");
  funcs_not_to_inst.insert("_condor_dprintf_va");
  funcs_not_to_inst.insert("__cxa_throw_bad_array_new_length");
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);

  if (getenv("SP_IPC")) {
    agent->EnableIpc(true);
  }
  
  StringSet preinst_funcs;
  preinst_funcs.insert("main");
  preinst_funcs.insert("exit");
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  agent->SetInitEvent(event);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("spi_test_exit");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {

}
