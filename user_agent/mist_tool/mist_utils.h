#ifndef _CHECKERUTILS_H_
#define _CHECKERUTILS_H_

#include <stack>
#include "SpInc.h"

namespace mist {

class TraceMgr;
class MistUtils {
  public:
    // Check whether function c has a name n, or has a name in ns
    static bool check_name(Dyninst::PatchAPI::PatchFunction* c, string n);
    static bool check_name(Dyninst::PatchAPI::PatchFunction* c,
                           std::vector<string> ns,
                           string* n=NULL);  // Return the name of c

    // Get user/group name from uid/gid
    static string get_user_name(uid_t id);
    static string get_group_name(gid_t id);

    // Manually manage call stack
    static void push(Dyninst::PatchAPI::PatchFunction* f);
    static Dyninst::PatchAPI::PatchFunction* pop();
    static void where();

    // print log by pid
    static void set_logfile(const char* fn);
    static void openlog();
    static void print(const char* fmt, ...);
    static void closelog();

    // Trace Management
    static void WriteHeader(std::string header);
    static void WriteTrace(std::string entry);
    static void WriteString(long pos,
                            std::string str);
    static void WriteString(std::string str);
    static void ChangeTraceFile();
    static void CloseTrace();
    static std::string TraceFileName();
    
    static unsigned long GetUsec();
  protected:
    typedef std::stack<Dyninst::PatchAPI::PatchFunction*> CallStack;
    static CallStack call_stack_;

    static string logfile_;
    static FILE* log_;
    static pid_t root_pid_;

    static TraceMgr mgr_;
};

}

#endif /* _CHECKERUTILS_H_ */
