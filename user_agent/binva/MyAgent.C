#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;


class BinVaChecker {
  public:
    virtual bool check(SpPoint* pt) = 0;
};

class DoubleFreeChecker : public BinVaChecker {
  public:
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;

      if (f->name().compare("free") == 0) {
	ArgumentHandle h;
	void** p = (void**)pop_argument(pt, &h, sizeof(void*));
	if (free_list_.find(*p) != free_list_.end()) {
	  sp_perror("double free %lx, skip it", *p);
	} else {
	  sp_print("free %lx", *p);
	  free_list_.insert(*p);
	}
      }
    }
  private:
    std::set<void*> free_list_;
};

class DangerousFuncChecker : public BinVaChecker {
  public:
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;

      bool dangerous = false;
      if (f->name().compare("getwd") == 0 ||
         f->name().compare("strcpy") == 0)||
         f->name().compare("strlen") == 0) {
	dangerous = true;
      }
      if (dangerous) {
	sp_print("Dangerous Function: %s", f->name().c_str());
      }
    }
};

class PrintfChecker : public BinVaChecker {
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

class BinVa {
  public:
    BinVa() {
      checkers_.insert(new DoubleFreeChecker);
      checkers_.insert(new DangerousFuncChecker);
      checkers_.insert(new PrintfChecker);
    }
    ~BinVa() {
      for (std::set<BinVaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	delete *i;
      }
    }
    void run(SpPoint* pt) {
      for (std::set<BinVaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->check(pt);
      }
    }
  private:
    std::set<BinVaChecker*> checkers_; 
};

BinVa bv;
void binva_head(SpPoint* pt) {

  bv.run(pt);

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
