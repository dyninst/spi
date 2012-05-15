#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

FILE* fp = NULL;
pid_t previous_pid = 0;

void test_entry(SpPoint* pt) {

  PatchFunction* f = Callee(pt);
  if (!f) return;
  fprintf(fp, "%s @ pid=%d\n",
          f->name().c_str(), getpid());

  SpPoint* caller = pt->caller_pt();
  fprintf(fp, "%s<-", f->name().c_str());
  while (caller) {
    PatchFunction* caller_func = Callee(caller);
    if (caller_func) {
      fprintf(fp, "%s<-", caller_func->name().c_str());
    } else {
      break;
    }
    caller = caller->caller_pt();
  }
  fprintf(fp, "main @ %d\n", getpid());
  /*
  if (previous_pid != getpid()) {
    char fn[255];
    snprintf(fn, 255, "/tmp/spi-%d-output", getpid());
    fp = fopen(fn, "w");
    previous_pid = getpid();
  }
  fprintf(fp, "%s() in %s @ pid=%d\n",
           f->name().c_str(),
           pt->GetObject()->name().c_str(),
           getpid());
  */
  sp::Propel(pt);
}

void spi_test_exit(SpPoint* pt) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  char fn[255];
  snprintf(fn, 255, "/tmp/spi-%d-output", getpid());
  fp = fopen(fn, "w");

  StringSet libs_to_inst;
  libs_to_inst.insert("libcondor_utils.so");
  libs_to_inst.insert("libclassad.so");
  libs_to_inst.insert("libvomsapi_gcc64dbg.so");
  agent->SetLibrariesToInstrument(libs_to_inst);

  StringSet funcs_not_to_inst;
  // funcs_not_to_inst.insert("ExprTreeToString");
  funcs_not_to_inst.insert("dc_main");
  /*
  funcs_not_to_inst.insert("DaemonCore::reconfig");
  funcs_not_to_inst.insert("config");
  funcs_not_to_inst.insert("check_core_files");
  funcs_not_to_inst.insert("MyString::~MyString");
  funcs_not_to_inst.insert("MyString::init");
  funcs_not_to_inst.insert("param_info_hash_lookup");
  funcs_not_to_inst.insert("param_boolean");
  funcs_not_to_inst.insert("DaemonCore::DaemonCore");
  funcs_not_to_inst.insert("drop_core_in_log");
  */
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
