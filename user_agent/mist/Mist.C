#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

Mist::Mist() {
  sp_print("=============================================");
  sp_print("========= PROC_INIT: start checking =========");
  sp_print("=============================================");

  // each init_checker is called only once when this agent is loaded
  init_checkers_.insert(new ProcInitChecker);

  // each checker is called for each point
  checkers_.insert(new FileOpenChecker);
  checkers_.insert(new LibChecker);
  checkers_.insert(new ChangeIdChecker);
  checkers_.insert(new ForkChecker);

  // each fini_checker is called only once when this agent is unloaded
  fini_checkers_.insert(new ProcFiniChecker);

  init_run();
}
Mist::~Mist() {
  fini_run();

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

  sp_print("=============================================");
  sp_print("========= PROC_FINI: finish checking ========");
  sp_print("=============================================");
}

void Mist::init_run() {
  // run init checkers
  for (std::set<InitChecker*>::iterator i = init_checkers_.begin(); i != init_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::fini_run() {
  // run fini checkers
  for (std::set<FiniChecker*>::iterator i = fini_checkers_.begin(); i != fini_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::run(SpPoint* pt, PatchFunction* f) {
  for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
    (*i)->check(pt, f);
  }
}

void Mist::post_run(SpPoint* pt, PatchFunction* f) {
  for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
    (*i)->post_check(pt, f);
  }
}
