#include "test_case.h"
#include "SpAgent.h"
#include "SpEvent.h"

using sp::SpParser;
using sp::SpAgent;
using sp::SyncEvent;

AGENT_INIT
void init_indcall() {
  dprint("AGENT: init_indcall @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();

  SpParser::ptr parser = SpParser::create();
  agent->set_parser(parser);

  SyncEvent::ptr event = SyncEvent::create();
  agent->set_init_event(event);

  agent->go();
}
