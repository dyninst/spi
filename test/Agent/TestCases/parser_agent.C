#include "test_case.h"
#include "SpAgent.h"
#include "SpEvent.h"

#include "frame.h"
#include "walker.h"
using Dyninst::Stackwalker::Walker;
using Dyninst::Stackwalker::Frame;

using sp::SpParser;
using sp::SpAgent;
using sp::SyncEvent;

AGENT_INIT
void init_parser() {
  dprint("AGENT: init_parser @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();

  SpParser::ptr parser = SpParser::create();
  agent->set_parser(parser);

  SyncEvent::ptr event = SyncEvent::create();
  agent->set_init_event(event);

  agent->set_init_payload("simple_payload");

  agent->go();

  Dyninst::PatchAPI::PatchFunction* mutatee_func = parser->findFunction("run_mutatee");
  assert(mutatee_func);
}
