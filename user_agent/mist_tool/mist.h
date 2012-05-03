#ifndef _MIST_H_
#define _MIST_H_

#include "SpInc.h"
#include "mist_checker.h"

namespace mist {

  typedef std::set<OneTimeChecker*> OneTimeCheckers;
  typedef std::set<MistChecker*> Checkers;
  
//--------------------------------------------------------
// The manager class to host checkers
//--------------------------------------------------------
class Mist {
  public:
    Mist();
    ~Mist();
    void init_run();
    void fini_run();
    void run(sp::SpPoint* pt,
             sp::SpFunction* f);
    void post_run(sp::SpPoint* pt,
                  sp::SpFunction* f);
  private:
    OneTimeCheckers init_checkers_; 
    Checkers checkers_; 
    OneTimeCheckers fini_checkers_;
    MistUtils u_;
};

}

#endif /* _MIST_H_ */
