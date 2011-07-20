#include "test_case.h"
#include "SpAgent.h"

using sp::SpParser;
using sp::SpAgent;

AGENT_INIT
void init_parser() {
  dprint("AGENT: init_parser @ process %d", getpid());

  SpParser::ptr parser = SpParser::create();
  SpAgent::ptr agent = SpAgent::create();
  agent->set_parser(parser);
  agent->go();

  Dyninst::ParseAPI::Function* mutatee_func = parser->findFunction("run_mutatee");
  assert(mutatee_func);
}
