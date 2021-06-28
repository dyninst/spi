#include <sys/stat.h>

#include "fpva.h"

namespace sp {
extern SpContext* g_context;
}

namespace fpva {

Fpva::Fpva() {
  this->mgr_ = new TraceMgr();
  // Each init_checker is called only once when this agent is loaded
  initCheckers_.push_back(new ProcInitChecker(mgr_));
  // Each checker is called for each point
  checkers_.push_back(new ForkChecker(mgr_));
  checkers_.push_back(new IpcChecker(mgr_));
}

Fpva::~Fpva() {
  FiniRun();

  // Clean up
  for (std::vector<OneTimeChecker*>::iterator i = initCheckers_.begin();
       i != initCheckers_.end(); i++) {
    delete *i;
  }

  for (std::vector<FpvaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
    delete *i;
  }

  for (std::vector<OneTimeChecker*>::iterator i = finiCheckers_.begin();
       i != finiCheckers_.end(); i++) {
    delete *i;
  }
}

void Fpva::InitRun() {
  // run only once for a process
  for (std::vector<OneTimeChecker*>::iterator i = initCheckers_.begin();
       i != initCheckers_.end(); i++) {
    (*i)->Run();
  }
}

void Fpva::FiniRun() {
  // run fini checkers
  for (std::vector<OneTimeChecker*>::iterator i = finiCheckers_.begin();
       i != finiCheckers_.end(); i++) {
    (*i)->Run();
  }
}

void Fpva::PreRun(sp::SpPoint* pt, sp::SpFunction* f) {
  static int i = 1;
  if (i) {
    InitRun();
    i = 0;
  }

  for (std::vector<FpvaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
    (*i)->PreCheck(pt, f);
  }
  mgr_->Flush();
}

void Fpva::PostRun(sp::PointHandle* pHandle) {
  for (std::vector<FpvaChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
    (*i)->PostCheck(pHandle);
  }
  mgr_->Flush();
}

}  // namespace mist
