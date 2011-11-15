#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include "MistUtils.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

CheckerUtils::CallStack CheckerUtils::call_stack_;
string CheckerUtils::logfile_;
FILE* CheckerUtils::log_ = NULL;
pid_t CheckerUtils::root_pid_ = 0;

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
  string s = "\n";

  while (!c.empty()) {
    s += "    => ";
    s += c.top()->name().c_str();
    s += "()\n";
    c.pop();
  }
  s += "    => main()";
  print("  - call stack: %s", s.c_str());
}

void CheckerUtils::set_logfile(const char* fn) {
  if (fn) {
    logfile_ = fn;
  }
}

void CheckerUtils::openlog() {
  // XXX: for now , we only print out info from the very first process
  root_pid_ = getpid();

  if (logfile_.size() > 0) {
    char buf[255];
    snprintf(buf, 255, "%s-%d", logfile_.c_str(), getpid());
    log_ = fopen(buf, "w+");
    assert(log_);
  } else {
    log_ = stderr;
  }
}

void CheckerUtils::print(const char* fmt, ...) {
  // XXX: should be fixed after adding support to multi-process
  if (getpid() != root_pid_) return;
 
  va_list ap;
  va_start(ap, fmt);
  vfprintf(log_, fmt, ap);
  vfprintf(log_, "\n", ap);
  fflush(log_);
  va_end(ap);
}

void CheckerUtils::closelog() {
  // XXX: should be fixed after adding support to multi-process
  if (getpid() != root_pid_) return;

  if (logfile_.size() > 0) {
    fclose(log_);
  }
}
