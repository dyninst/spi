#ifndef _MISTCHECKER_H_
#define _MISTCHECKER_H_

#include "SpInc.h"
#include "mist_utils.h"

namespace mist {
//-----------------------------------------
// Base classes
//-----------------------------------------
class OneTimeChecker {
  public:
    virtual bool run() = 0;
    virtual ~OneTimeChecker() {}
  protected:
    MistUtils u_;
};

class MistChecker {
  public:
  virtual ~MistChecker() {}
  virtual bool check(sp::SpPoint* pt,
                     sp::SpFunction* callee) = 0;
  virtual bool post_check(sp::SpPoint* pt,
                          sp::SpFunction* callee) = 0;
  protected:
    MistUtils u_;
};

//-----------------------------------------
// Derived classes
//-----------------------------------------

// Check process stat
class ProcInitChecker : public OneTimeChecker {
  public:
    virtual bool run();
  protected:
    // flag, privilege level
    // fork / exec, control flow, discuss w/ karl
    void PrintUserInfo();
    void PrintCurrentProc();
    void PrintParentProc();
    void PrintProc(pid_t pid);
    void PrintEnv();

    void InitTraces();
};


class ProcFiniChecker : public OneTimeChecker {
  public:
    virtual bool run() {
			return true;
    }
};

#if 0
// File opened
class FileOpenChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
  protected:
    void print_file_info(char* fname);
    char* fname_;
};

// Dynamic loaded library
class LibChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee) { return false;}
};

// Fork
class ForkChecker : public MistChecker {
  public:
    ForkChecker();
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
  private:
    std::vector<string> ns_;
};

// Check the changes of uid/gid
class ChangeIdChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee) { return false; }
};

// Check when to exit
class ExitChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
};

// Check mmap
class MmapChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
};

// Check chmod
class ChmodChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee) { return false; }
};

// Thread
class ThreadChecker : public MistChecker {
  public:
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
  protected:
    // A primitive form of concurrency control
    typedef std::map<long, pthread_t*> CalleeTidMap;
    CalleeTidMap callee_tid_map_;
};

#endif

// IPC
class IpcChecker : public MistChecker {
  public:
  virtual ~IpcChecker() {}
    virtual bool check(sp::SpPoint* pt,
                       sp::SpFunction* callee);
    virtual bool post_check(sp::SpPoint* pt,
                            sp::SpFunction* callee);
  protected:
    // trg host ip -> trg port -> protocol -> size count
    typedef std::map<int, size_t> ProtoSizeMap;
    typedef std::map<int, ProtoSizeMap> PidProtoMap;
    typedef std::map<std::string, PidProtoMap> SizeCountMap;
    SizeCountMap size_count_map_;
};

}
#endif /* _MISTCHECKER_H_ */
