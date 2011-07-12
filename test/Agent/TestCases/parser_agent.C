#include "test_case.h"
#include "Agent.h"

using sp::Parser;
using sp::Agent;

__attribute__((constructor))
void init_parser() {
  dprint("AGENT: init_parser @ process %d", getpid());

  Parser::ptr parser = Parser::create();
  Agent::ptr agent = Agent::create();
  agent->setParser(parser);
  agent->go();

  Dyninst::ParseAPI::Function* mutatee_func = parser->findFunction("run_mutatee");
  assert(mutatee_func);
}
