#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

SpLock g_lock;

void test_entry(SpPoint* pt) {
  //  SetSegfaultSignal();

  Lock(&g_lock);
	PatchFunction* f = Callee(pt);
  StringSet ss;
  
  if (!f) {
    goto PAYLOAD_EXIT;
  }

  if (getenv("SP_SKIP")) {
  ss.insert("new(");
  ss.insert("delete(");
  ss.insert("std::allocator");
  ss.insert("std::");
  ss.insert("__gnu_cxx::");
  // ss.insert("CommandLine::InitFromArgv");
  }

  for (StringSet::iterator i = ss.begin(); i != ss.end(); i++) {
    if (f->name().find(*i) != std::string::npos) {
      // sp_print("In %s, skipped", f->name().c_str());
      goto PAYLOAD_EXIT;
    }
  }

	sp_print("%s, tid=%ld", f->name().c_str(), (long)GetThreadId());
  sp::Propel(pt);
PAYLOAD_EXIT:
  Unlock(&g_lock);
  return;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  InitLock(&g_lock);
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
