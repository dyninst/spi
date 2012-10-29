#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int level = 0;
void test_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  for (int i = 0; i < level; i++) fprintf(stdout, " ");
  sp_print("%s", f->name().c_str());
  level ++;
  sp::Propel(pt);
}

void test_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;
  level--;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");

  agent->Go();
}
