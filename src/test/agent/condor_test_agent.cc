#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

FILE* fp = NULL;

void test_entry(SpPoint* pt) {

  PatchFunction* f = Callee(pt);
  if (!f) return;

  fprintf(stderr, "%s() in %s @ pid=%d\n",
           f->name().c_str(),
           pt->GetObject()->name().c_str(),
           getpid());
  
  if (IsIpcWrite(pt) || IsIpcRead(pt)) {
    sp_print("%s() in %s @ pid=%d",
             f->name().c_str(),
             pt->GetObject()->name().c_str(),
             getpid());
    /*
    fprintf(fp, "%s() in %s @ pid=%d",
             f->name().c_str(),
             pt->GetObject()->name().c_str(),
             getpid());
    */
  }
  sp::Propel(pt);
}

void spi_test_exit(SpPoint* pt) {
  PatchFunction* f = Callee(pt);
  if (!f) return;
  if (IsIpcWrite(pt) || IsIpcRead(pt)) {
    sp_print("Exit %s @ pid = %d",
             f->name().c_str(), getpid());
    /*
    fprintf(fp, "Exit %s @ pid = %d",
             f->name().c_str(), getpid());
    */
  }
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  // char fn[255];
  // snprintf(fn, 255, "/tmp/spi-%d", getpid());
  // fp = fopen(fn, "w");
  
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
  preinst_funcs.insert("main");
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  agent->SetInitEvent(event);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("spi_test_exit");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
