#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_entry(SpPoint* pt) {

	PatchFunction* f = callee(pt);
  if (!f) return;

  StringSet ss;
  ss.insert("MallocHook::InvokeDeleteHook");
  ss.insert("chrome_main::LowLevelInit");
  ss.insert("CommandLine::Init");

  for (StringSet::iterator i = ss.begin(); i != ss.end(); i++) {
    if ((*i).compare(f->name().c_str()) == 0) {
      sp_print("In %s, skipped", (*i).c_str());
      return;
    }
  }
  
	sp_print("%s", f->name().c_str());
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
  agent->Go();
}

AGENT_FINI
void DumpOutput() {
}
