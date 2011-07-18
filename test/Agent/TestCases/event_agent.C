#include "test_case.h"
#include "Agent.h"

using sp::Parser;
using sp::Agent;

AGENT_INIT
void init_event() {
  dprint("AGENT: init_event @ process %d", getpid());

  Agent::ptr agent = Agent::create();
  agent->go();
}
