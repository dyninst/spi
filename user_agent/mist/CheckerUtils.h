#ifndef _CHECKERUTILS_H_
#define _CHECKERUTILS_H_

#include <stack>
#include "SpInc.h"

class CheckerUtils {
  public:
    bool check_name(Dyninst::PatchAPI::PatchFunction* c, string n);
    bool check_name(Dyninst::PatchAPI::PatchFunction* c, std::vector<string> ns,
                    string* n=NULL);
    string get_user_name(uid_t id);
    string get_group_name(gid_t id);
    static void push(Dyninst::PatchAPI::PatchFunction* f);
    static Dyninst::PatchAPI::PatchFunction* pop();
    void where();
  protected:
    typedef std::stack<Dyninst::PatchAPI::PatchFunction*> CallStack;
    static CallStack call_stack_;
};


#endif /* _CHECKERUTILS_H_ */
