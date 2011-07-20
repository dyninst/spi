#include <signal.h>

#include "test_case.h"
#include "SpAgent.h"

using sp::SpParser;
using sp::SpAgent;
using sp::TimerEvent;

AGENT_INIT
void init_event() {
  dprint("AGENT: init_event @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();
  agent->set_init_event(TimerEvent::create(5));
  agent->go();
}
