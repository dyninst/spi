#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

//--------------------------------------------------------
// Various checkers
//--------------------------------------------------------
class CheckerUtils {
  public:
    static bool check_name(PatchFunction* c, string n) {
      return (c->name().compare(n) == 0);
    }
    static bool check_name(PatchFunction* c, std::vector<string> ns, string* n=NULL) {
      for (int i = 0; i < ns.size(); i++) {
	if (check_name(c, ns[i])) {
	  if (n) *n = ns[i];
	  return true;
	}
      }
      return false;
    }
};

class MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) = 0;
  protected:
    CheckerUtils u_;
};

class InitChecker {
  public:
    virtual bool run() = 0;
  protected:
    CheckerUtils u_;
};

class FiniChecker {
  public:
    virtual bool run() = 0;
  protected:
    CheckerUtils u_;
};

// Check process stat
class ProcInitChecker : public InitChecker {
  public:
    virtual bool run() {
      sp_print("  * PROC_INIT: start checking");

      // hard working!
      pid_ = getpid();

      // print out
      sp_print("    - pid: %d", pid_);
    }
  protected:
    pid_t pid_;
    // flag, print process / executable, arv, privilege level , user id, group id,
    // change group id, fork / exec, control flow, discuss w/ karl
};

class ProcFiniChecker : public FiniChecker {
  public:
    virtual bool run() {
      sp_print("  * PROC_FINI: finish checking");
    }
};

// File opened
class FileOpenChecker : public MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) {
      std::vector<string> ns;
      ns.push_back("fopen");
      ns.push_back("open");

      if (u_.check_name(callee, ns)) {
	ArgumentHandle h;
	char** fname = (char**)pop_argument(pt, &h, sizeof(void*));
	sp_print("    - FILE OPENED: %s", *fname);
      }
    }
};

//--------------------------------------------------------
// The manager class to host checkers
//--------------------------------------------------------
class Mist {
  public:
    Mist() {
      sp_print("=== ENTER PROCESS ===");

      // each init_checker is called only once when this agent is loaded
      init_checkers_.insert(new ProcInitChecker);

      // each checker is called for each point
      checkers_.insert(new FileOpenChecker);

      // each fini_checker is called only once when this agent is unloaded
      fini_checkers_.insert(new ProcFiniChecker);

      // run init checkers
      for (std::set<InitChecker*>::iterator i = init_checkers_.begin(); i != init_checkers_.end(); i++) {
	(*i)->run();
      }
    }
    ~Mist() {
      // run fini checkers
      for (std::set<FiniChecker*>::iterator i = fini_checkers_.begin(); i != fini_checkers_.end(); i++) {
	(*i)->run();
      }

      // clean up
      for (std::set<InitChecker*>::iterator i = init_checkers_.begin(); i != init_checkers_.end(); i++) {
	delete *i;
      }

      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	delete *i;
      }

      for (std::set<FiniChecker*>::iterator i = fini_checkers_.begin(); i != fini_checkers_.end(); i++) {
	delete *i;
      }

      sp_print("=== EXIT PROCESS ===");
    }
    void run(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return;

      sp_print("  * CHECKING %s", f->name().c_str());
      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->check(pt, f);
      }
    }
  private:
    std::set<InitChecker*> init_checkers_; 
    std::set<MistChecker*> checkers_; 
    std::set<FiniChecker*> fini_checkers_; 
};

//--------------------------------------------------------
// Standard self-propelled stuffs
//--------------------------------------------------------
Mist mist;
void mist_head(SpPoint* pt) {
  mist.run(pt);
  sp::propel(pt);
}

void mist_tail(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;
  sp_print("FINISH %s", f->name().c_str());
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);
  agent->set_init_head("mist_head");
  // agent->set_init_tail("mist_tail");

  agent->go();
}
