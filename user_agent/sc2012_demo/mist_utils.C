#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <sys/time.h>

#include "mist_utils.h"
#include "trace_mgr.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

namespace mist {

MistUtils::CallStack MistUtils::call_stack_;
string MistUtils::logfile_;
FILE* MistUtils::log_ = NULL;
pid_t MistUtils::root_pid_ = 0;
TraceMgr MistUtils::mgr_;

//////////////////////////////////////////////////////////////////////

bool
MistUtils::check_name(PatchFunction* c, string n) {
  return (c->name().compare(n) == 0);
}

//////////////////////////////////////////////////////////////////////

bool
MistUtils::check_name(PatchFunction* c,
                      std::vector<string> ns,
                      string* n) {
  for (unsigned i = 0; i < ns.size(); i++) {
    if (check_name(c, ns[i])) {
      if (n) *n = ns[i];
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

string
MistUtils::get_user_name(uid_t id) {
  struct passwd* p;
  if((p = getpwuid(id)) != NULL )
    return p->pw_name;
  return "";
}

//////////////////////////////////////////////////////////////////////

string
MistUtils::get_group_name(gid_t id) {
  struct group* g = getgrgid(id);
  if (g)  return g->gr_name;
  return "";
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::push(PatchFunction* f) {
  call_stack_.push(f);
}

//////////////////////////////////////////////////////////////////////

PatchFunction*
MistUtils::pop() {
  PatchFunction* f = call_stack_.top();
  call_stack_.pop();
  return f;
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::where() {
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

//////////////////////////////////////////////////////////////////////

void
MistUtils::set_logfile(const char* fn) {
  if (fn) {
    logfile_ = fn;
  }
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::openlog() {
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

//////////////////////////////////////////////////////////////////////

void
MistUtils::print(const char* fmt, ...) {
  // XXX: should be fixed after adding support to multi-process
  if (getpid() != root_pid_) return;

  va_list ap;
  va_start(ap, fmt);
  vfprintf(log_, fmt, ap);
  vfprintf(log_, "\n", ap);
  fflush(log_);
  va_end(ap);
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::closelog() {
  // XXX: should be fixed after adding support to multi-process
  if (getpid() != root_pid_) return;

  if (logfile_.size() > 0) {
    fclose(log_);
  }
}

// -------------------------------------------------------------------
// Trace Management
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

void
MistUtils::WriteHeader(std::string entry) {
  mgr_.WriteHeader(entry);
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::WriteTrace(std::string entry) {
  mgr_.WriteTrace(entry);
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::WriteString(std::string str) {
  mgr_.WriteString(str);
}

//////////////////////////////////////////////////////////////////////

void
MistUtils::WriteString(long pos,
                       std::string str) {
  mgr_.WriteString(pos, str);
}

void
MistUtils::CloseTrace() {
  mgr_.CloseTrace();
}

void
MistUtils::ChangeTraceFile() {
  mgr_.ChangeTraceFile();
}

std::string
MistUtils::TraceFileName() {
  return mgr_.TraceFileName();
}

// ------------------------------------------------------------------- 
// Get usecs since 1970
// -------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////

unsigned long MistUtils::GetUsec() {
  struct timeval tv;
  int ret = gettimeofday(&tv, NULL);
  if (ret == 0) {
    return ((tv.tv_sec - 1339166869l)* 1000000 + tv.tv_usec);
  }
  return 0;
}

}
