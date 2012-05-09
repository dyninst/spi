#include <sys/stat.h>
#include <sys/mman.h>

#include "mist.h"
#include "mist_checker.h"
#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

namespace mist {

//------------------------------------------------
// Check process stat
//------------------------------------------------

//////////////////////////////////////////////////

bool
ProcInitChecker::run() {
  PrintCurrentProc();
  PrintUserInfo();
  PrintEnv();
  PrintParentProc();

  InitTraces();
	return true;
}

//////////////////////////////////////////////////

void
ProcInitChecker::PrintUserInfo() {
  char entry[1024];

  // Real user
  snprintf(entry, 1024,
           "<real_user><name>%s</name><id>%d</id></real_user>",
           u_.get_user_name(getuid()).c_str(),
           getuid());
  u_.WriteHeader(entry);

  // Effective user
  snprintf(entry, 1024,
           "<effective_user><name>%s</name><id>%d</id></effective_user>",
           u_.get_user_name(geteuid()).c_str(),
           geteuid());
  u_.WriteHeader(entry);
  
  // Real group
  snprintf(entry, 1024,
           "<real_group><name>%s</name><id>%d</id></real_group>",
           u_.get_group_name(getgid()).c_str(),
           getgid());
  u_.WriteHeader(entry);

  // Effective group
  snprintf(entry, 1024,
           "<effective_group><name>%s</name><id>%d</id></effective_group>",
           u_.get_group_name(getegid()).c_str(),
           getegid());
  u_.WriteHeader(entry);
}

//////////////////////////////////////////////////

void
ProcInitChecker::PrintCurrentProc() {
  
  PrintProc(getpid());
  char entry[1024];
  
  // Get working dir
  char buf[256];
  if (getcwd(buf, 256) != NULL) {
    snprintf(entry, 1024, "<working_dir>%s</working_dir>", buf);
    u_.WriteHeader(entry);
  }
  
  // Host ip
  sp::GetIPv4Addr(buf, 256);
  snprintf(entry, 1024, "<host>%s</host>", buf);
  u_.WriteHeader(entry);
}

//////////////////////////////////////////////////

// Assumption: it is the last one to be called in ProcInitChecker::run()
void
ProcInitChecker::PrintParentProc() {
  char entry[1024];
  snprintf(entry, 1024, "<parent>");
  u_.WriteHeader(entry);
  
  PrintProc(getppid());

  snprintf(entry, 1024, "</parent>");
  u_.WriteHeader(entry);
}

//////////////////////////////////////////////////

// XXX: should make it elegant!
void
ProcInitChecker::PrintProc(pid_t pid) {
  char path[255];
  snprintf(path, 255, "/proc/%d/cmdline", pid);
  FILE* fp = fopen(path, "r");
  int c;

  char entry[1024];

  // Get pid
  snprintf(entry, 1024, "<pid>%d</pid>", getpid());
  u_.WriteHeader(entry);
  
  // Get exe
  string exe;
  while ((c = fgetc(fp)) != '\0') {
    exe += (char)c;
  }
  exe += '\0';

  snprintf(entry, 1024, "<exe_name>%s</exe_name>", exe.c_str());
  u_.WriteHeader(entry);

  // Get arguments
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
  for (unsigned i = 0; i < args.size(); i++) {
    cmdline += " ";
    cmdline += args[i].c_str();
  }
  snprintf(entry, 1024,
           "<cmdline_argument>%s</cmdline_argument>",
           cmdline.c_str());
  u_.WriteHeader(entry);
  
  fclose(fp);
}

//////////////////////////////////////////////////

void
ProcInitChecker::PrintEnv() {

  char entry[1024];
  snprintf(entry, 1024, "<environment_vars>");
  u_.WriteHeader(entry);

  // Get environment variables
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

  int count = 1;
  for (Envs::iterator i = envs.begin(); i != envs.end(); i++, count++) {
    snprintf(entry, 1024, "<%s value=\"%s\" />",
             (*i).first.c_str(), (*i).second.c_str());
    u_.WriteHeader(entry);
  }

  snprintf(entry, 1024, "</environment_vars>");
  u_.WriteHeader(entry);
}

//////////////////////////////////////////////////

// Assumption for the current layout of xml file:
// <?xml ... ?><process><head></head></process>
void
ProcInitChecker::InitTraces() {
  u_.WriteString(-10, "<traces></traces></process>");
}

#if 0
//------------------------------------------------
// File opened
//------------------------------------------------
bool FileOpenChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("fopen");
  ns.push_back("open");

  if (u_.check_name(callee, ns)) {
    ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    u_.print("* FILE OPENED: %s", *fname);
    fname_ = (char*)*fname;
  }
	return true;
}

bool FileOpenChecker::post_check(sp::SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("fopen");
  ns.push_back("open");

  if (u_.check_name(callee, ns)) {
    int fd = 0;
    if (u_.check_name(callee, "open")) {
      fd = ReturnValue(pt);
    } else {
      fd = fileno(((FILE*)ReturnValue(pt)));
    }
    u_.print("  - file descriptor: fd = %d", fd);
    print_file_info(fname_);
    u_.where();
  }
  return false;
}

void FileOpenChecker::print_file_info(char* fname) {
  struct stat s;
  if (stat(fname, &s) == -1) {
    sp_perror("FAILED TO stat %s", fname);
    return;
  }
  mode_t m = s.st_mode;
  u_.print("  - number of hard links: %d", s.st_nlink);
  u_.print("  - owned by: (name: %s, uid: %d) in (group: %s, gid: %d)",
	   u_.get_user_name(s.st_uid).c_str(), s.st_uid,
	   u_.get_group_name(s.st_gid).c_str(), s.st_gid);
  u_.print("  - owner perm: %c%c%c", (S_IRUSR&m)?'r':'-', (S_IWUSR&m)?'w':'-', (S_IXUSR&m)?'x':'-');
  u_.print("  - group perm: %c%c%c", (S_IRGRP&m)?'r':'-', (S_IWGRP&m)?'w':'-', (S_IXGRP&m)?'x':'-');
  u_.print("  - other perm: %c%c%c", (S_IROTH&m)?'r':'-', (S_IWOTH&m)?'w':'-', (S_IXOTH&m)?'x':'-');
  u_.print("  - setuid bit: %s", (S_ISUID&m)?"set":"unset");
}

// Dynamic loaded library
bool LibChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("dlopen");

  if (u_.check_name(callee, ns)) {
    ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    u_.print("* LIBRARY LOADED: %s", *fname);
    
    // print this file's info
    struct stat s;
    if (stat(*fname, &s) == -1) return false;
    mode_t m = s.st_mode;
    u_.print("  - owned by: (name: %s, uid: %d) in (group: %s, gid: %d)",
	     u_.get_user_name(s.st_uid).c_str(), s.st_uid,
	     u_.get_group_name(s.st_gid).c_str(), s.st_gid);
    u_.print("  - owner perm: %c%c%c", (S_IRUSR&m)?'r':'-', (S_IWUSR&m)?'w':'-', (S_IXUSR&m)?'x':'-');
    u_.print("  - group perm: %c%c%c", (S_IRGRP&m)?'r':'-', (S_IWGRP&m)?'w':'-', (S_IXGRP&m)?'x':'-');
    u_.print("  - other perm: %c%c%c", (S_IROTH&m)?'r':'-', (S_IWOTH&m)?'w':'-', (S_IXOTH&m)?'x':'-');
    u_.print("  - setuid bit: %s", (S_ISUID&m)?"set":"unset");

    // stack backtrace
    u_.where();
  }
	return true;
}

// Check fork
ForkChecker::ForkChecker() {
  ns_.push_back("fork");
}

bool ForkChecker::check(SpPoint* pt, PatchFunction* callee) {
	return true;
}

bool ForkChecker::post_check(SpPoint* pt, PatchFunction* callee) {
  if (u_.check_name(callee, ns_)) {
    long child_pid = ReturnValue(pt);
    if (child_pid) {
      u_.print("* FORK: parent id - %d, child id - %d", getpid(), child_pid);
      u_.where();
    }
  }
  return false;
}

// check uid/gid changes
// Check the changes of uid/gid
bool ChangeIdChecker::check(SpPoint* pt, PatchFunction* callee) {
  bool found = false;
  ArgumentHandle h;
  uid_t* first = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
  uid_t* second = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
  uid_t* third = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));

  string s = "";
  char buf[512];
  if (u_.check_name(callee, "setuid")) {
    snprintf(buf, 512, "  - real user: (name: %s, id: %d)", u_.get_user_name(*first).c_str(), *first);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "seteuid")) {
    snprintf(buf, 512, "  - effect user: (name: %s, id: %d)", u_.get_user_name(*first).c_str(), *first);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setreuid")) {
    snprintf(buf, 512, "  - real user: (name: %s, id: %d)\n", u_.get_user_name(*first).c_str(), *first);
    s += buf;
    snprintf(buf, 512, "  - effect user: (name: %s, id: %d)", u_.get_user_name(*second).c_str(), *second);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setresuid")) {
    snprintf(buf, 512, "  - real user: (name: %s, id: %d)\n", u_.get_user_name(*first).c_str(), *first);
    s += buf;
    snprintf(buf, 512, "  - effect user: (name: %s, id: %d)\n", u_.get_user_name(*second).c_str(), *second);
    s += buf;
    snprintf(buf, 512, "  - saved user: (name: %s, id: %d)", u_.get_user_name(*third).c_str(), *third);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setgid")) {
    snprintf(buf, 512, "  - real group: (name: %s, id: %d)", u_.get_group_name(*first).c_str(), *first);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setegid")) {
    snprintf(buf, 512, "  - effect group: (name: %s, id: %d)", u_.get_group_name(*first).c_str(), *first);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setregid")) {
    snprintf(buf, 512, "  - real group: (name: %s, id: %d)\n", u_.get_group_name(*first).c_str(), *first);
    s += buf;
    snprintf(buf, 512, "  - effect group: (name: %s, id: %d)", u_.get_group_name(*second).c_str(), *second);
    s += buf;
    found = true;
  } else if (u_.check_name(callee, "setresgid")) {
    snprintf(buf, 512, "  - real group: (name: %s, id: %d)\n", u_.get_group_name(*first).c_str(), *first);
    s += buf;
    snprintf(buf, 512, "  - effect group: (name: %s, id: %d)\n", u_.get_group_name(*second).c_str(), *second);
    s += buf;
    snprintf(buf, 512, "  - saved group: (name: %s, id: %d)", u_.get_group_name(*third).c_str(), *third);
    s += buf;
    found = true;
  }

  if (found) {
    u_.print("* ID changed: %s", callee->name().c_str());
    u_.print("%s", s.c_str());
    u_.where();
  }
	return true;
}

// Exit checker
extern Mist mist;
bool ExitChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("exit");

  if (u_.check_name(callee, ns)) {
    ArgumentHandle h;
    int* exit_code = (int*)PopArgument(pt, &h, sizeof(void*));
    u_.print("* EXIT: w/ status code %d", *exit_code);
    u_.where();
    // mist.fini_run();
  }
	return true;
}

bool ExitChecker::post_check(sp::SpPoint* pt, PatchFunction* callee) {
  return false;
}

// mmap checker
bool MmapChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("mmap");

  if (u_.check_name(callee, ns)) {
    ArgumentHandle h;
    void** addr = (void**)PopArgument(pt, &h, sizeof(void*));
    size_t* len = (size_t*)PopArgument(pt, &h, sizeof(size_t));
    int* prot = (int*)PopArgument(pt, &h, sizeof(int));
    int* flag = (int*)PopArgument(pt, &h, sizeof(int));
    int* fd = (int*)PopArgument(pt, &h, sizeof(int));
    off_t* offset = (off_t*)PopArgument(pt, &h, sizeof(off_t));

    u_.print("* MMAP: (addr - %lx, len - %d) ", *addr, *len);
    string prot_str;
    if (*prot & PROT_NONE) prot_str += "PROT_NONE ";
    if (*prot & PROT_READ) prot_str += "PROT_READ ";
    if (*prot & PROT_WRITE) prot_str += "PROT_WRITE ";
    if (*prot & PROT_EXEC) prot_str += "PROT_EXEC ";
    u_.print("  - protection: %s (%x)", prot_str.c_str(), *prot);

    string flag_str;
    if (*flag & MAP_FIXED) flag_str += "MAP_FIXED ";
    if (*flag & MAP_SHARED) flag_str += "MAP_SHARED ";
    if (*flag & MAP_PRIVATE) flag_str += "MAP_PRIVATE ";
    if (*flag & MAP_ANONYMOUS) flag_str += "MAP_ANONYMOUS ";
    if (*flag & MAP_LOCKED) flag_str += "MAP_LOCKED ";
    u_.print("  - flags: %s (%x)", flag_str.c_str(), *flag);

    struct stat s;
    size_t fsize = 0;
    if (fstat(*fd, &s) != -1) {
      fsize = s.st_size;
    }

    u_.print("  - backed file: (fd - %d, offset - %d, file size - %d)",
            *fd, *offset, fsize);
  }
	return true;
}

bool MmapChecker::post_check(sp::SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("mmap");

  if (u_.check_name(callee, ns)) {
    long start_addr = ReturnValue(pt);
    u_.print("  - actual mapped address: %lx", start_addr);
    u_.where();
    return true;
  }
	return true;
}


// chmod checker
bool ChmodChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("chmod");

  if (u_.check_name(callee, ns)) {
    //ArgumentHandle h;
    //int* exit_code = (int*)PopArgument(pt, &h, sizeof(void*));
    //u_.print("* EXIT: w/ status code %d", *exit_code);
    u_.print("* CHMOD:");
    u_.where();
  }
	return true;
}

// thread checker
bool ThreadChecker::check(SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("pthread_create");

  if (u_.check_name(callee, ns)) {
    ArgumentHandle h;
    pthread_t** tid = (pthread_t**)PopArgument(pt, &h, sizeof(pthread_t*));
    callee_tid_map_[(long)callee] = *tid; 
   }
  return true;
}

bool ThreadChecker::post_check(sp::SpPoint* pt, PatchFunction* callee) {
  std::vector<string> ns;
  ns.push_back("pthread_create");

  if (u_.check_name(callee, ns)) {
    pthread_t t = *callee_tid_map_[(long)callee];
    u_.print("* PTHREAD_CREATE: new thread id %d", t);
    u_.print("  - created by: (pid: %d, tid: %d)", getpid(), pthread_self());
    u_.where();
   }
	return true;
}

#endif

// ------------------------------------------------------------------- 
// Ipc Checker
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

bool
IpcChecker::check(SpPoint* pt,
                  SpFunction* callee) {
  return true;
}

//////////////////////////////////////////////////////////////////////
// TODO:
// 1. maintain mapping: pid->tid->trg_pid->tid size counter on exit
// 2. output trace on exit

bool
IpcChecker::post_check(SpPoint* pt,
                       SpFunction* callee) {

  bool ipc = false;
  size_t size = 0;
  
	if (IsIpcWrite(pt)) {
    ipc = true;
    size = sp::ReturnValue(pt);
    if (size == 0) return true;
    u_.WriteTrace("<trace type=\"channel_write\">");
	}
	else if (IsIpcRead(pt)) {
    ipc = true;
    size = sp::ReturnValue(pt);
    if (size == 0) return true;
    u_.WriteTrace("<trace type=\"channel_read\">");
	}

  if (ipc) {
    sp::TcpChannel* channel = static_cast<sp::TcpChannel*>(pt->channel());
    if (channel) {
      std::string remote_addr = channel->GetRemoteHost();
      if (remote_addr.compare("127.0.0.1") == 0) {
        char buf[256];
        sp::GetIPv4Addr(buf, 256);
        remote_addr = buf;
      }
      int remote_port = channel->GetRemotePort();
      size_t& total_size = size_count_map_[remote_addr][remote_port][channel->type];
      total_size += size;
      std::string mechanism = "tcp";

      char buf[1024];
      snprintf(buf, 1024,
               "<target>"
               "<host>%s</host>"
               "<port>%d</port>"
               "<pid>%d</pid>"
               "</target>"
               "<size>%lu</size>"
               "<total_size>%lu</total_size>"
               "<mechanism>%s</mechanism>"
               "<call>%s</call>",
               remote_addr.c_str(), remote_port,
               channel->GetRemotePid(),
               size, total_size,
               mechanism.c_str(), callee->name().c_str());
      u_.WriteTrace(buf);
    }
    u_.WriteTrace("</trace>");
  }
	return true;
}

}
