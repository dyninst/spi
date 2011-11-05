#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

class MistChecker {
  public:
    virtual bool check(SpPoint* pt) = 0;
};

class TestChecker : public MistChecker {
  public:
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;
      if (f->name().compare("printf") == 0) {
	ArgumentHandle h;
	char** fmt = (char**)pop_argument(pt, &h, sizeof(void*));
	sp_print("Printf format string: %s", *fmt);
      }
    }
};

class Mist {
  public:
    Mist() {
      checkers_.insert(new TestChecker);
    }
    ~Mist() {
      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	delete *i;
      }
    }
    void run(SpPoint* pt) {
      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->check(pt);
      }
    }
  private:
    std::set<MistChecker*> checkers_; 
};

Mist mist;
void binva_head(SpPoint* pt) {

  mist.run(pt);

  sp::propel(pt);
}

void binva_tail(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);

  agent->set_init_head("binva_head");
  // agent->set_init_tail("binva_tail");

  agent->go();
}
