#include "SpInc.h"
#include "Function.h"
#include "Type.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

class BinVaChecker {
  public:
    virtual bool check(SpPoint* pt) = 0;
    virtual bool post_check(SpPoint* pt) = 0; 
    virtual bool fini_report() = 0;
};

class DoubleFreeChecker : public BinVaChecker {
  public:
    virtual bool fini_report() {}
    virtual bool post_check(SpPoint* pt) {}
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
    virtual bool fini_report() {}
    virtual bool post_check(SpPoint* pt) {}
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;

      bool dangerous = false;
      if (f->name().compare("getwd") == 0 ||
         f->name().compare("strcpy") == 0 ||
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
    virtual bool fini_report() {}
    virtual bool post_check(SpPoint* pt) {}
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

// Bookkeeping for stack array, for future checking on buffer overflow problem
class StackArrayChecker : public BinVaChecker {
  public:
    virtual bool fini_report() {}
    virtual bool post_check(SpPoint* pt) {}
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;

      sp_print("StackArrayChecker");
      // getAllVariables
      PatchObject* obj = pt->obj();
      using namespace Dyninst::ParseAPI;
      using namespace Dyninst::SymtabAPI;
      SymtabCodeSource* cs = static_cast<SymtabCodeSource*>(obj->co()->cs());
      Symtab* sym = cs->getSymtabObject();
      std::vector<Symbol*> symbols;

      std::vector<SymtabAPI::Function*> funcs;
      sym->getAllFunctions(funcs);

      for (int i = 0; i < funcs.size(); i ++) {
	if (funcs[i]->getOffset() == f->addr()) {

	  std::vector<localVar*> vars;
	  funcs[i]->getLocalVariables(vars);

	  for (int j = 0; j < vars.size(); j ++) {
	    typeArray* t = vars[j]->getType()->getArrayType();
	    if (!t) continue;
	    sp_print("%s: [%lx, %lx]", vars[j]->getName().c_str(), t->getLow(), t->getHigh());
	  }
	}
      }
    }
};

class MallocFreeChecker : public BinVaChecker {
  public:
    virtual bool fini_report() {
      if (MemSet.size() != 0) {
	sp_print("Unbalanced malloc/free pairs");
	for (std::set<void*>::iterator i = MemSet.begin(); i != MemSet.end(); i++) {
	  sp_print("- unfreed pointer: %lx", (*i));
	}
      }
    }
    virtual bool post_check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;

      if (f->name().compare("malloc") == 0) {
	void* p = (void*)retval(pt);
	MemSet.insert(p);
	sp_print("malloc: %lx", p);
      }
    }
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;
      if (f->name().compare("free") == 0) {
	ArgumentHandle h;
	void** p = (void**)pop_argument(pt, &h, sizeof(void*));
	if (MemSet.find(*p) == MemSet.end()) {
	  sp_print("freeing an unmalloced pointer: %lx", *p);
	} else {
	  MemSet.erase(*p);
	  // sp_print("free: %lx", *p);
	}
      }
    }
  protected:
    static std::set<void*> MemSet;
};
std::set<void*> MallocFreeChecker::MemSet;

class FileAccessChecker : public BinVaChecker {
  public:
    virtual bool fini_report() {}
    virtual bool post_check(SpPoint* pt) {}
    virtual bool check(SpPoint* pt) {
      PatchFunction* f = sp::callee(pt);
      if (!f) return false;
      if (f->name().compare("open") == 0) {
	ArgumentHandle h;
	char** fn = (char**)pop_argument(pt, &h, sizeof(void*));
	sp_print("Open: %s", *fn);
      }
    }
};

class BinVa {
  public:
    BinVa() {
      checkers_.insert(new DoubleFreeChecker);
      checkers_.insert(new DangerousFuncChecker);
      checkers_.insert(new PrintfChecker);
      checkers_.insert(new MallocFreeChecker);
      checkers_.insert(new FileAccessChecker);
      checkers_.insert(new StackArrayChecker);
    }
    ~BinVa() {
      for (std::set<BinVaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->fini_report();
	delete *i;
      }
    }
    void run(SpPoint* pt) {
      for (std::set<BinVaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->check(pt);
      }
    }
    void post_run(SpPoint* pt) {
      for (std::set<BinVaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->post_check(pt);
      }
    }
  private:
    std::set<BinVaChecker*> checkers_; 
};

BinVa bv;
void binva_before(SpPoint* pt) {
  bv.run(pt);
  sp::propel(pt);
}

void binva_tail(SpPoint* pt) {
  bv.post_run(pt);
  sp::propel(pt);
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);

  agent->set_init_before("binva_before");
  agent->set_init_after("binva_after");

  agent->go();
}
