#include "test_case.h"
#include "SpInc.h"

using sp::SpParser;
using sp::SpAgent;
using sp::SyncEvent;

AGENT_INIT
void init_indcall() {
  dprint("AGENT: init_indcall @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();
  // agent->set_directcall_only(true);

  agent->go();
}
