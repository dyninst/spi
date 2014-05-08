/*
  It will output trace data to /tmp. Please read README file for more details.
 */

#include "SpInc.h"
#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

mist::Mist g_mist;

void mist_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;
  g_mist.run(pt, f);
  sp::Propel(pt);
}

void mist_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  g_mist.post_run(pt, f);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;  
  sp_print("INSTRUMENTATION(pid=%d): libagent.so loaded in %s\n",
          getpid(), sp::GetExeName().c_str());
  libs_to_inst.insert("mod_chunked.so");
  libs_to_inst.insert("libcondor_utils_7_9_4.so");
  libs_to_inst.insert("libclassad.so.7.9.4");
  libs_to_inst.insert("libvomsapi_gcc64dbg.so");
  libs_to_inst.insert("libc-2.15.so");
  libs_to_inst.insert("libm-2.15.so");
  agent->SetLibrariesToInstrument(libs_to_inst); 
  agent->SetInitEntry("mist_entry");
  agent->SetInitExit("mist_exit");
  agent->EnableIpc(true);
  agent->Go();
}
