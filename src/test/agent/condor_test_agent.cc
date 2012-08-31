#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

FILE* fp = NULL;
pid_t previous_pid = 0;

void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  sp_print("%s", f->GetMangledName().c_str());

  sp::Propel(pt);
}

void spi_test_exit(SpPoint* pt) {
}

AGENT_INIT
void MyAgent() {
#if 0
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  StringSet libs_to_inst;
  libs_to_inst.insert("libcondor_utils.so");
  libs_to_inst.insert("libclassad.so");
  libs_to_inst.insert("libvomsapi_gcc64dbg.so");
  agent->SetLibrariesToInstrument(libs_to_inst);

  StringSet funcs_not_to_inst;
  // funcs_not_to_inst.insert("ExprTreeToString");
  /*
  funcs_not_to_inst.insert("classad::ClassAdUnParser::ClassAdUnParser");
  funcs_not_to_inst.insert("_ZNSs6assignEPKcm");
  funcs_not_to_inst.insert("classad::ClassAdUnParser::SetOldClassAd");
  funcs_not_to_inst.insert("classad::ClassAdUnParser::Unparse");
  funcs_not_to_inst.insert("classad::ClassAdUnParser::~ClassAdUnParser");
  funcs_not_to_inst.insert("__cxa_guard_acquire");
  funcs_not_to_inst.insert("__cxa_guard_release");
  funcs_not_to_inst.insert("__cxa_atexit");
  */
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);

  if (getenv("SP_IPC")) {
    agent->EnableIpc(true);
  }
  
  StringSet preinst_funcs;
  preinst_funcs.insert("main");
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  agent->SetInitEvent(event);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("spi_test_exit");
  agent->Go();
#endif
}

AGENT_FINI
void DumpOutput() {

}
