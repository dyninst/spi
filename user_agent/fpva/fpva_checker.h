#ifndef _FPVACHECKER_H_
#define _FPVACHECKER_H_

#include "SpInc.h"
#include "fpva_utils.h"
#include "trace_mgr.h"

namespace fpva {
//-----------------------------------------
// Base classes
//-----------------------------------------
class OneTimeChecker {
 public:
  OneTimeChecker(TraceMgr* mgr);
  virtual bool Run() = 0;
  virtual ~OneTimeChecker() {}

 protected:
  TraceMgr* mgr_;
};

class FpvaChecker {
 public:
  FpvaChecker(TraceMgr* mgr);
  virtual ~FpvaChecker() {}
  virtual bool PreCheck(sp::SpPoint* pt, sp::SpFunction* callee) = 0;
  virtual bool PostCheck(sp::PointCallHandle* pHandle) = 0;

 protected:
  TraceMgr* mgr_;
};

//-----------------------------------------
// Derived classes
//-----------------------------------------

// Check process stat
class ProcInitChecker : public OneTimeChecker {
 public:
  using OneTimeChecker::OneTimeChecker;
  bool Run();
  ~ProcInitChecker() {}
};

// IPC
class IpcChecker : public FpvaChecker {
 public:
  using FpvaChecker::FpvaChecker;
  virtual ~IpcChecker() {}
  bool PreCheck(sp::SpPoint* pt, sp::SpFunction* callee);
  bool PostCheck(sp::PointCallHandle* pHandle);

 protected:
  string port;
};

class ForkChecker : public FpvaChecker {
 public:
  using FpvaChecker::FpvaChecker;
  bool PreCheck(sp::SpPoint* pt, sp::SpFunction* callee);
  bool PostCheck(sp::PointCallHandle* pHandle);
};

class FileChecker : public FpvaChecker {
 public:
  using FpvaChecker::FpvaChecker;
  bool PreCheck(sp::SpPoint* pt, sp::SpFunction* callee);
  bool PostCheck(sp::PointCallHandle* pHandle);
};

class PrivilegeChecker : public FpvaChecker {
 public:
  using FpvaChecker::FpvaChecker;
  bool PreCheck(sp::SpPoint* pt, sp::SpFunction* callee);
  bool PostCheck(sp::PointCallHandle* pHandle);
};

}  // namespace fpva
#endif /* _FPVACHECKER_H_ */
