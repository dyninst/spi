#include "mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

namespace mist {

Mist::Mist() {

  // Each init_checker is called only once when this agent is loaded
  init_checkers_.insert(new ProcInitChecker);

  // Each checker is called for each point
  checkers_.insert(new IpcChecker);
  /*
  checkers_.insert(new FileOpenChecker);
  checkers_.insert(new LibChecker);
  checkers_.insert(new ChangeIdChecker);
  checkers_.insert(new ForkChecker);
  checkers_.insert(new ExitChecker);
  checkers_.insert(new MmapChecker);
  checkers_.insert(new ChmodChecker);
  checkers_.insert(new ThreadChecker);
  */  
  // Each fini_checker is called only once when this agent is unloaded
  fini_checkers_.insert(new ProcFiniChecker);

  init_run();
}

Mist::~Mist() {
  fini_run();

  // Clean up
  for (OneTimeCheckers::iterator i = init_checkers_.begin();
       i != init_checkers_.end(); i++) {
    delete *i;
  }

  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    delete *i;
  }

  for (OneTimeCheckers::iterator i = fini_checkers_.begin();
       i != fini_checkers_.end(); i++) {
    delete *i;
  }
}

void Mist::fork_init_run() {
  u_.ChangeTraceFile();
  init_run();
}

void Mist::init_run() {
  // run init checkers
  for (OneTimeCheckers::iterator i = init_checkers_.begin();
       i != init_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::fini_run() {
  // run fini checkers
  for (OneTimeCheckers::iterator i = fini_checkers_.begin();
       i != fini_checkers_.end(); i++) {
    (*i)->run();
  }
}

void Mist::run(SpPoint* pt, SpFunction* f) {
  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    (*i)->check(pt, f);
  }
}

void Mist::post_run(SpPoint* pt, SpFunction* f) {
  for (Checkers::iterator i = checkers_.begin();
       i != checkers_.end(); i++) {
    (*i)->post_check(pt, f);
  }
}

}
