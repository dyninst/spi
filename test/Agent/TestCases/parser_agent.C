#include "test_case.h"
#include "Agent.h"
/*
#include "frame.h"
#include "walker.h"
*/
using sp::Parser;
using sp::Agent;
//using namespace Dyninst::Stackwalker;

AGENT_INIT
void init_parser() {
  dprint("AGENT: init_parser @ process %d", getpid());

  Parser::ptr parser = Parser::create();
  Agent::ptr agent = Agent::create();
  agent->setParser(parser);
  agent->go();
  /*
  Dyninst::ParseAPI::Function* mutatee_func = parser->findFunction("run_mutatee");
  assert(mutatee_func);

  std::vector<Frame> stackwalk; 
  string s;
  Walker *walker = Walker::newWalker();
  walker->walkStack(stackwalk);
  for (unsigned i=0; i<stackwalk.size(); i++) {
    stackwalk[i].getName(s);
    cout << "Found function " << s << endl;
  }
  */
}
