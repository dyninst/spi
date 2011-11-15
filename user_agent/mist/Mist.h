#ifndef _MIST_H_
#define _MIST_H_

#include "SpInc.h"
#include "MistChecker.h"

//--------------------------------------------------------
// The manager class to host checkers
//--------------------------------------------------------
class Mist {
  public:
    Mist();
    ~Mist();
    void init_run();
    void fini_run();
    void run(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* f);
    void post_run(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* f);
  private:
    std::set<InitChecker*> init_checkers_; 
    std::set<MistChecker*> checkers_; 
    std::set<FiniChecker*> fini_checkers_;
    CheckerUtils u_;
};


#endif /* _MIST_H_ */
