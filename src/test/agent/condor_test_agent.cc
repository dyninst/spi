#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


void test_entry(SpPoint* pt) {

  PatchFunction* f = Callee(pt);
  if (!f) return;

  if (IsIpcWrite(pt) || IsIpcRead(pt)) {

    sp_print("%s() in %s @ pid=%d",
             f->name().c_str(),
             pt->GetObject()->name().c_str(),
             getpid());
  }
  sp::Propel(pt);
}

void spi_test_exit(SpPoint* pt) {
  PatchFunction* f = Callee(pt);
  if (!f) return;
  if (IsIpcWrite(pt) || IsIpcRead(pt)) {
    sp_print("Exit %s @ pid = %d",
             f->name().c_str(), getpid());
  }
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  StringSet libs_to_inst;
  libs_to_inst.insert("libcondor_utils.so");
  libs_to_inst.insert("libclassad.so");
  libs_to_inst.insert("libvomsapi_gcc64dbg.so");
  agent->SetLibrariesToInstrument(libs_to_inst);

  StringSet funcs_not_to_inst;
  funcs_not_to_inst.insert("ExprTreeToString");
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);

  if (getenv("SP_IPC")) {
    agent->EnableIpc(true);
  }
  
  StringSet preinst_funcs;
  preinst_funcs.insert("Daemon::connectSock");
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  agent->SetInitEvent(event);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("spi_test_exit");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
