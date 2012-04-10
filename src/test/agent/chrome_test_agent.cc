#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

SpLock g_lock;
FILE* fp;

typedef std::set<SpPoint*> UniqCallSites;
UniqCallSites uniq_call_sites;

typedef std::set<PatchFunction*> UniqFuncs;
UniqFuncs uniq_funcs;

unsigned long all_calls = 0;

void test_entry(SpPoint* pt) {
  SetSegfaultSignal();

	PatchFunction* f = Callee(pt);
  if (!f) return;

  if (std::find(uniq_funcs.begin(), uniq_funcs.end(), f)
      == uniq_funcs.end()) {
    fprintf(fp, "%s\n", f->name().c_str());
    uniq_funcs.insert(f);
  }
  /*
  uniq_call_sites.insert(pt);
  all_calls++;

	fprintf(fp, "uniq_call_sites: %ld, uniq_funcs: %ld, all calls: %ld\n",
          uniq_call_sites.size(), uniq_funcs.size(), all_calls);
  */
  sp::Propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();

  char logname[255];
  sprintf(logname, "/scratch/wenbin/spi%d", getpid());
  fp = fopen(logname, "w+");
  StringSet ss;
  ss.insert("new(");
  ss.insert("delete(");
  ss.insert("std::allocator");
  ss.insert("std::");
  ss.insert("__gnu_cxx::");
  ss.insert("CommandLine::InitFromArgv");
  ss.insert("tc_");

  sp::StringSet preinst_func;
  preinst_func.insert("main");
  preinst_func.insert("base::CreateThread");
  preinst_func.insert("base::PlatformThread::Create");
  preinst_func.insert("base::LaunchProcess");
  sp::FuncEvent::ptr event = sp::FuncEvent::Create(preinst_func);
  
  agent->SetFuncsNotToInstrument(ss);
  agent->SetInitEntry("test_entry");
  agent->SetInitEvent(event);
  //agent->EnableIpc(true);
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
