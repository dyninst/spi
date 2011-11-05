#include "test_case.h"
#include "SpAgent.h"
#include "SpEvent.h"
using sp::SpParser;
using sp::SpAgent;
using sp::SyncEvent;

void param_head(Dyninst::PatchAPI::Point* pt) {
  Dyninst::PatchAPI::PatchFunction* f = sp::callee(pt);
  sp::ArgumentHandle h;

  printf("got %s\n", f->name().c_str());

  if (f && f->name().compare("func1") == 0) {
    int* a = (int*)sp::pop_argument(pt, &h, sizeof(int));
    printf("%s - Argument 1: %d\n", f->name().c_str(), *a);
  }
  else if (f && f->name().compare("func2") == 0) {
    int* a = (int*)sp::pop_argument(pt, &h, sizeof(int));
    int* c = (int*)sp::pop_argument(pt, &h, sizeof(int));

    printf("%s - Argument 1: %d, Argument 2: %d\n", f->name().c_str(), *a, *c);
  }
  else if (f && f->name().compare("func") == 0) {
    int* a = (int*)sp::pop_argument(pt, &h, sizeof(int));
    char** b = (char**)sp::pop_argument(pt, &h, sizeof(char*));
    printf("%s - Argument 1: %d, Argument 2: %s\n", f->name().c_str(), *a, (*b));
  }
  sp::propel(pt);
}

AGENT_INIT
void init_param() {
  dprint("AGENT: init_param @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();
  agent->set_init_head("param_head");
  agent->go();
  //Dyninst::PatchAPI::PatchFunction* mutatee_func = parser->findFunction("run_mutatee");
  //assert(mutatee_func);
}
