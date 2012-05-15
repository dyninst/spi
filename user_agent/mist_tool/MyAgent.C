#include "SpInc.h"
#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

mist::Mist g_mist;

// ------------------------------------------------------------------- 
// Condor process hierarchy
// -------------------------------------------------------------------
void condor_hier_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  g_mist.run(pt, f);
}

void condor_hier_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

  if (f->name().compare("fork") == 0) {
    long ret = ReturnValue(pt);
    if (ret == 0) {
      g_mist.fork_init_run();
    }
  }
  
  g_mist.post_run(pt, f);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  StringSet funcs_not_to_inst;
  StringSet preinst_funcs;

  // For Condor process hierarchy
  agent->SetInitEntry("condor_hier_entry");
  agent->SetInitExit("condor_hier_exit");
  libs_to_inst.insert("libcondor_utils.so");

  // For fork()
  preinst_funcs.insert("main");
  preinst_funcs.insert("Server::ProcessServiceReq");
  preinst_funcs.insert("Server::Replicate");
  preinst_funcs.insert("Server::ProcessStoreReq");
  preinst_funcs.insert("Server::ProcessRestoreReq");
  preinst_funcs.insert("CreateProcessForkit::fork_exec");
  preinst_funcs.insert("DaemonCore::Create_Thread");
  preinst_funcs.insert("dc_main");
  preinst_funcs.insert("privsep_launch_switchboard");
  preinst_funcs.insert("cpuusage_test");
  preinst_funcs.insert("recursive_fork");
  preinst_funcs.insert("fork_tree");
  preinst_funcs.insert("test_monitor");
  preinst_funcs.insert("glexec_kill");
  preinst_funcs.insert("UserProc::execute");
  preinst_funcs.insert("load_avg_test");
  preinst_funcs.insert("SSHToJob::execute_ssh");
  preinst_funcs.insert("email_open_implementation");
  preinst_funcs.insert("ForkWorker::Fork");
  preinst_funcs.insert("my_popenv_impl");
  preinst_funcs.insert("my_spawnv");
  preinst_funcs.insert("spawn");
  preinst_funcs.insert("fool_child");
  preinst_funcs.insert("Workers::createWorkers");
  preinst_funcs.insert("systemCommand");
  preinst_funcs.insert("midwife_executable");
  preinst_funcs.insert("quit_on_signal_with_core");
  preinst_funcs.insert("safe_is_path_trusted_fork");

  // Fork connect()
  preinst_funcs.insert("tcp_connect");
  preinst_funcs.insert("Sock::connect");
  preinst_funcs.insert("SafeSock::connect");
  preinst_funcs.insert("ReliSock::connect_socketpair");
  preinst_funcs.insert("SharedPortClient::PassSocket");
  preinst_funcs.insert("open_tcp_stream");
  preinst_funcs.insert("open_tcp");
  preinst_funcs.insert("SSHToJob::receiveSshConnection");
  preinst_funcs.insert("condor_connect");
  
  agent->SetFuncsNotToInstrument(funcs_not_to_inst);
  agent->SetLibrariesToInstrument(libs_to_inst);
  FuncEvent::ptr event = FuncEvent::Create(preinst_funcs);
  agent->SetInitEvent(event);
  agent->EnableIpc(false);
  agent->Go();
}
