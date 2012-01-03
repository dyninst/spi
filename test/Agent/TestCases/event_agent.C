#include <signal.h>

#include "test_case.h"
#include "SpInc.h"

using sp::SpParser;
using sp::SpAgent;
using sp::AsyncEvent;

AGENT_INIT
void init_event() {
  dprint("AGENT: init_event @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();
  agent->set_init_event(AsyncEvent::create(SIGALRM, 5));
  agent->go();
}
