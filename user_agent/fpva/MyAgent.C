/*
  It will output trace data to .../tmp. Please read README file for more
  details.
 */

#include "SpInc.h"
#include "fpva.h"

fpva::Fpva* G_FPVA;

void FpvaEntry(sp::SpPoint* pt) {
  sp::SpFunction* f = Callee(pt);
  if (!f) return;
  G_FPVA->PreRun(pt, f);
  sp::Propel(pt);
}

void FpvaExit(sp::PointHandle* pHandle) {
  sp::SpFunction* f = pHandle->GetCallee();
  if (!f) return;
  G_FPVA->PostRun(pHandle);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  sp_print("INSTRUMENTATION(pid=%d): libagent.so loaded in %s\n", getpid(),
           sp::GetExeName().c_str());
  sp::StringSet libs_not_to_inst{
      "linux-vdso.so",  "libdl.so",           "libresolv.so",
      "librt.so",       "libcrypto.so",       "libstdc++.so",
      "libm.so",        "libgomp.so",         "libpthread.so",
      "libc.so",        "ld-linux-x86-64.so", "libparseAPI.so",
      "libpcontrol.so", "libstackwalk.so",    "libpatchAPI.so"};
  agent->SetLibrariesNotToInstrument(libs_not_to_inst);

  sp::StringSet funcs_not_to_inst;
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

  agent->SetInitEntry("FpvaEntry");
  agent->SetInitExit("FpvaExit");
  agent->EnableIpc(true);
  agent->Go();
  G_FPVA = new fpva::Fpva();
}
