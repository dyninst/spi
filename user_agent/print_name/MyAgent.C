#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int level = 0;
void test_entry(PointCallHandle* handle) {
  SpFunction* f = handle->GetCallee();
  SpPoint* pt = handle->GetPoint();
  if (!f) return;

  for (int i = 0; i < level; i++) fprintf(stdout, " ");
  fprintf(stdout, "%s\n", f->name().c_str());
  level ++;
  sp::Propel(pt);
}

void test_exit(PointCallHandle* handle) {
  SpFunction* f = handle->GetCallee();
  if (!f) return;
  level--;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");

  agent->Go();
}
