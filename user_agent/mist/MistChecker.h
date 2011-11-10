#ifndef _MISTCHECKER_H_
#define _MISTCHECKER_H_

#include "SpInc.h"
#include "CheckerUtils.h"

//-----------------------------------------
// Base classes
//-----------------------------------------
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

class MistChecker {
  public:
  virtual bool check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee) = 0;
  virtual bool post_check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee) = 0;
  protected:
    CheckerUtils u_;
};

//-----------------------------------------
// Derived classes
//-----------------------------------------

// Check process stat
class ProcInitChecker : public InitChecker {
  public:
    virtual bool run();
  protected:
    // flag, privilege level
    // fork / exec, control flow, discuss w/ karl
    void print_user_info();
    void print_current_proc();
    void print_parent_proc();
    void print_proc(pid_t pid);
    void print_env();
};


class ProcFiniChecker : public FiniChecker {
  public:
    virtual bool run() {
    }
};

// File opened
class FileOpenChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee);
    virtual bool post_check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee) { return false;}
  protected:
    void print_file_info(char* fname);
};

// Dynamic loaded library
class LibChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee);
    virtual bool post_check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee) { return false;}
};

// Fork
class ForkChecker : public MistChecker {
  public:
    ForkChecker();
    virtual bool check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee);
    virtual bool post_check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee);
  private:
    std::vector<string> ns_;
};

// Check the changes of uid/gid
class ChangeIdChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee);
    virtual bool post_check(sp::SpPoint* pt, Dyninst::PatchAPI::PatchFunction* callee) { return false; }
};

#endif /* _MISTCHECKER_H_ */
