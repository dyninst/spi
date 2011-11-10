#include <grp.h>
#include <pwd.h>
#include "CheckerUtils.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

CheckerUtils::CallStack CheckerUtils::call_stack_;

bool CheckerUtils::check_name(PatchFunction* c, string n) {
  return (c->name().compare(n) == 0);
}

bool CheckerUtils::check_name(PatchFunction* c, std::vector<string> ns, string* n) {
  for (int i = 0; i < ns.size(); i++) {
    if (check_name(c, ns[i])) {
      if (n) *n = ns[i];
      return true;
    }
  }
  return false;
}

string CheckerUtils::get_user_name(uid_t id) {
  struct passwd* p = getpwuid(id);
  return p->pw_name;
}
string CheckerUtils::get_group_name(gid_t id) {
  struct group* g = getgrgid(id);
  return g->gr_name;
}

void CheckerUtils::push(PatchFunction* f) {
  call_stack_.push(f);
}

PatchFunction* CheckerUtils::pop() {
  PatchFunction* f = call_stack_.top();
  call_stack_.pop();
  return f;
}

void CheckerUtils::where() {
  CallStack c = call_stack_;
  string s;
  while (!c.empty()) {
    s += c.top()->name().c_str();
    s += " <= ";
    c.pop();
  }
  s += "main";
  sp_print("  - call stack: %s", s.c_str());
}
