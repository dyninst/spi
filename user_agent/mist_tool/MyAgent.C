/*
  It will output trace data to /tmp. Please read README file for more details.
 */

#include "SpInc.h"
#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

mist::Mist g_mist;

void condor_hier_entry(SpPoint* pt) {

  SpFunction* f = Callee(pt);
  if (!f) return;

  g_mist.run(pt, f);

  sp::Propel(pt);
}

void condor_hier_exit(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return;

  g_mist.post_run(pt, f);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_to_inst;
  agent->SetLibrariesToInstrument(libs_to_inst);
  agent->EnableIpc(true);
  agent->Go();
}
