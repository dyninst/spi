#include "SpInc.h"
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <stack>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

//--------------------------------------------------------
// Various checkers
//--------------------------------------------------------
class CheckerUtils {
  public:
    bool check_name(PatchFunction* c, string n) {
      return (c->name().compare(n) == 0);
    }
    bool check_name(PatchFunction* c, std::vector<string> ns, string* n=NULL) {
      for (int i = 0; i < ns.size(); i++) {
	if (check_name(c, ns[i])) {
	  if (n) *n = ns[i];
	  return true;
	}
      }
      return false;
    }
    string get_user_name(uid_t id) {
      struct passwd* p = getpwuid(id);
      return p->pw_name;
    }
    string get_group_name(gid_t id) {
      struct group* g = getgrgid(id);
      return g->gr_name;
    }

    static void push(PatchFunction* f) {
      call_stack_.push(f);
    }
    static PatchFunction* pop() {
      PatchFunction* f = call_stack_.top();
      call_stack_.pop();
      return f;
    }
    void where() {
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
  protected:
    typedef std::stack<PatchFunction*> CallStack;
    static CallStack call_stack_;
};
CheckerUtils::CallStack CheckerUtils::call_stack_;

class MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) = 0;
  protected:
    CheckerUtils u_;
};

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

// Check process stat
class ProcInitChecker : public InitChecker {
  public:
    virtual bool run() {
      print_parent_proc();
      print_current_proc();
      print_user_info();
      // print_env();
    }

  protected:
    // flag, privilege level
    // fork / exec, control flow, discuss w/ karl

    void print_user_info() {
      sp_print("  - real user: (name: %s, id: %d)",
              u_.get_user_name(getuid()).c_str(), getuid());
      sp_print("  - effective user: (name: %s, id: %d)",
              u_.get_user_name(geteuid()).c_str(), geteuid());
      sp_print("  - real group: (name: %s, id: %d)",
	       u_.get_group_name(getgid()).c_str(), getgid());
     sp_print("  - effective group: (name: %s, id: %d)",
               u_.get_group_name(getegid()).c_str(), getegid());
    }

    void print_current_proc() {
      sp_print("* current process");
      print_proc(getpid());
      char buf[256];
      getcwd(buf, 256);
      sp_print("  - working dir: %s", buf);
    }

    void print_parent_proc() {
      sp_print("* parent process");
      print_proc(getppid());
    }

    // XXX: should make it elegant!
    void print_proc(pid_t pid) {
      char path[255];
      sp_print("  - pid: %d", pid);
      snprintf(path, 255, "/proc/%d/cmdline", pid);
      FILE* fp = fopen(path, "r");
      int c;
      // get exe
      string exe;
      while ((c = fgetc(fp)) != '\0') {
	exe += (char)c;
      }
      exe += '\0';
      sp_print("  - exe name: %s", exe.c_str());

      // get arguments
      typedef std::vector<string> Args;
      Args args;
      while (c == '\0' || isprint(c)) {
	string arg;
	while ((c = fgetc(fp)) != '\0' && isprint(c)) {
	  arg += (char)c;
	}
	if (arg.size() > 0) {
  	  arg += '\0';
	  args.push_back(arg);
	}
      }
      string cmdline;
      for (int i = 0; i < args.size(); i++) {
	cmdline += " ";
	cmdline += args[i].c_str();
      }
      sp_print("  - cmdline: %s", cmdline.c_str());
      fclose(fp);
    }

    void print_env() {
      // get environment variables
      typedef std::map<string, string> Envs;
      Envs envs;
      char** e = environ;
      while (*e) {
	char* head = *e;
        char* tail = strchr(*e, '=');
	string h(head, tail-head);
	string t = (tail+1);
	envs[h] = t;
      	e++;
      }
      sp_print("* environment variables");
      int count = 1;
      for (Envs::iterator i = envs.begin(); i != envs.end(); i++, count++) {
	sp_print("  %d: %s = %s", count, (*i).first.c_str(), (*i).second.c_str());
      }
    }
};

class ProcFiniChecker : public FiniChecker {
  public:
    virtual bool run() {
    }
};

// File opened
class FileOpenChecker : public MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) {
      std::vector<string> ns;
      ns.push_back("fopen");
      ns.push_back("open");

      if (u_.check_name(callee, ns)) {
	ArgumentHandle h;
	char** fname = (char**)pop_argument(pt, &h, sizeof(void*));
	sp_print("* FILE OPENED: %s", *fname);
	print_file_info(*fname);
	u_.where();
      }
    }
  protected:
    void print_file_info(char* fname) {
      struct stat s;
      // XXX
      if (stat(fname, &s) == -1) return;
      mode_t m = s.st_mode;
      sp_print("  - owned by: (name: %s, uid: %d) in (group: %s, gid: %d)",
	       u_.get_user_name(s.st_uid).c_str(), s.st_uid,
               u_.get_group_name(s.st_gid).c_str(), s.st_gid);
      sp_print("  - owner perm: %c%c%c", (S_IRUSR&m)?'r':'-', (S_IWUSR&m)?'w':'-', (S_IXUSR&m)?'x':'-');
      sp_print("  - group perm: %c%c%c", (S_IRGRP&m)?'r':'-', (S_IWGRP&m)?'w':'-', (S_IXGRP&m)?'x':'-');
      sp_print("  - other perm: %c%c%c", (S_IROTH&m)?'r':'-', (S_IWOTH&m)?'w':'-', (S_IXOTH&m)?'x':'-');
      sp_print("  - setuid bit: %s", (S_ISUID&m)?"set":"unset");
    }
};

// Dynamic loaded library
class LibChecker : public MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) {
      std::vector<string> ns;
      ns.push_back("dlopen");

      if (u_.check_name(callee, ns)) {
	ArgumentHandle h;
	char** fname = (char**)pop_argument(pt, &h, sizeof(void*));
	sp_print("* LIBRARY LOADED: %s", *fname);
      }
    }
};

class ChangeIdChecker : public MistChecker {
  public:
    virtual bool check(SpPoint* pt, PatchFunction* callee) {

      sp_print("* ID changed: %s", callee->name().c_str());
      u_.where();
      ArgumentHandle h;
      uid_t* first = (uid_t*)pop_argument(pt, &h, sizeof(uid_t));
      uid_t* second = (uid_t*)pop_argument(pt, &h, sizeof(uid_t));
      uid_t* third = (uid_t*)pop_argument(pt, &h, sizeof(uid_t));

      string s = "";
      char buf[512];
      if (u_.check_name(callee, "setuid")) {
	snprintf(buf, 512, "  - real user: (name: %s, id: %d)", u_.get_user_name(*first).c_str(), *first);
	s += buf;
      } else if (u_.check_name(callee, "seteuid")) {
	snprintf(buf, 512, "  - effect user: (name: %s, id: %d)", u_.get_user_name(*first).c_str(), *first);
	s += buf;
      } else if (u_.check_name(callee, "setreuid")) {
	snprintf(buf, 512, "  - real user: (name: %s, id: %d)\n", u_.get_user_name(*first).c_str(), *first);
	s += buf;
	snprintf(buf, 512, "  - effect user: (name: %s, id: %d)", u_.get_user_name(*second).c_str(), *second);
	s += buf;
      } else if (u_.check_name(callee, "setresuid")) {
	snprintf(buf, 512, "  - real user: (name: %s, id: %d)\n", u_.get_user_name(*first).c_str(), *first);
	s += buf;
	snprintf(buf, 512, "  - effect user: (name: %s, id: %d)\n", u_.get_user_name(*second).c_str(), *second);
	s += buf;
	snprintf(buf, 512, "  - saved user: (name: %s, id: %d)", u_.get_user_name(*third).c_str(), *third);
	s += buf;
      } else if (u_.check_name(callee, "setgid")) {
	snprintf(buf, 512, "  - real group: (name: %s, id: %d)", u_.get_group_name(*first).c_str(), *first);
	s += buf;
      } else if (u_.check_name(callee, "setegid")) {
	snprintf(buf, 512, "  - effect group: (name: %s, id: %d)", u_.get_group_name(*first).c_str(), *first);
	s += buf;
      } else if (u_.check_name(callee, "setregid")) {
	snprintf(buf, 512, "  - real group: (name: %s, id: %d)\n", u_.get_group_name(*first).c_str(), *first);
	s += buf;
	snprintf(buf, 512, "  - effect group: (name: %s, id: %d)", u_.get_group_name(*second).c_str(), *second);
	s += buf;
      } else if (u_.check_name(callee, "setresgid")) {
	snprintf(buf, 512, "  - real group: (name: %s, id: %d)\n", u_.get_group_name(*first).c_str(), *first);
	s += buf;
	snprintf(buf, 512, "  - effect group: (name: %s, id: %d)\n", u_.get_group_name(*second).c_str(), *second);
	s += buf;
	snprintf(buf, 512, "  - saved group: (name: %s, id: %d)", u_.get_group_name(*third).c_str(), *third);
	s += buf;
      }


      sp_print("%s", s.c_str());
    }
  protected:
    void internal(SpPoint* pt, PatchFunction* callee) {
    }
};

//--------------------------------------------------------
// The manager class to host checkers
//--------------------------------------------------------
class Mist {
  public:
    Mist() {
      sp_print("=============================================");
      sp_print("========= PROC_INIT: start checking =========");
      sp_print("=============================================");

      // each init_checker is called only once when this agent is loaded
      init_checkers_.insert(new ProcInitChecker);

      // each checker is called for each point
      checkers_.insert(new FileOpenChecker);
      checkers_.insert(new LibChecker);
      checkers_.insert(new ChangeIdChecker);

      // each fini_checker is called only once when this agent is unloaded
      fini_checkers_.insert(new ProcFiniChecker);

      // run init checkers
      for (std::set<InitChecker*>::iterator i = init_checkers_.begin(); i != init_checkers_.end(); i++) {
	(*i)->run();
      }
    }
    ~Mist() {
      // run fini checkers
      for (std::set<FiniChecker*>::iterator i = fini_checkers_.begin(); i != fini_checkers_.end(); i++) {
	(*i)->run();
      }

      // clean up
      for (std::set<InitChecker*>::iterator i = init_checkers_.begin(); i != init_checkers_.end(); i++) {
	delete *i;
      }

      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	delete *i;
      }

      for (std::set<FiniChecker*>::iterator i = fini_checkers_.begin(); i != fini_checkers_.end(); i++) {
	delete *i;
      }
      sp_print("=============================================");
      sp_print("========= PROC_FINI: finish checking ========");
      sp_print("=============================================");
    }
    void run(SpPoint* pt, PatchFunction* f) {
      for (std::set<MistChecker*>::iterator i = checkers_.begin(); i != checkers_.end(); i++) {
	(*i)->check(pt, f);
      }
    }
  private:
    std::set<InitChecker*> init_checkers_; 
    std::set<MistChecker*> checkers_; 
    std::set<FiniChecker*> fini_checkers_; 
};

//--------------------------------------------------------
// Standard self-propelled stuffs
//--------------------------------------------------------
Mist mist;
void mist_head(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  CheckerUtils::push(f);
  mist.run(pt, f);
  if (pt->tailcall()) {
    CheckerUtils::pop();
  }
  sp::propel(pt);
}

void mist_tail(SpPoint* pt) {
  CheckerUtils::pop();
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);
  agent->set_init_head("mist_head");
  agent->set_init_tail("mist_tail");
  agent->go();

  sp_print("=============================================");
  sp_print("========= CHECKING: for each call ===========");
  sp_print("=============================================");
}
