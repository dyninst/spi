#ifndef _FPVA_H_
#define _FPVA_H_

#include "SpInc.h"
#include "fpva_checker.h"

namespace fpva {
//--------------------------------------------------------
// The manager class to host checkers
//--------------------------------------------------------
class Fpva {
 public:
  Fpva();
  ~Fpva();
  void InitRun();
  void FiniRun();
  void PreRun(sp::SpPoint* pt, sp::SpFunction* f);
  void PostRun(sp::PointCallHandle* pHandle);

 private:
  std::vector<OneTimeChecker*> initCheckers_;
  std::vector<OneTimeChecker*> finiCheckers_;
  std::vector<FpvaChecker*> checkers_;
  TraceMgr* mgr_;
};

}  // namespace fpva

#endif /* _FPVA_H_ */
