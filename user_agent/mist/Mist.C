#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

Mist::Mist() {

  // each init_checker is called only once when this agent is loaded
  init_checkers_.insert(new ProcInitChecker);

  // each checker is called for each point
  checkers_.insert(new FileOpenChecker);
  checkers_.insert(new LibChecker);
  checkers_.insert(new ChangeIdChecker);
  checkers_.insert(new ForkChecker);
  checkers_.insert(new ExitChecker);
  checkers_.insert(new MmapChecker);
  checkers_.insert(new ChmodChecker);
  checkers_.insert(new ThreadChecker);

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
}

void Mist::init_run() {
  if (getenv("MIST_LOG")) {
    u_.set_logfile("../../x86_64-unknown-linux2.4/splog");
  } else {
    u_.set_logfile(NULL);
  }

  u_.openlog();

  u_.print("=============================================");
  u_.print("========= PROC_INIT: start checking =========");
  u_.print("=============================================");

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
  u_.print("=============================================");
  u_.print("========= PROC_FINI: finish checking ========");
  u_.print("=============================================");
  u_.closelog();
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
