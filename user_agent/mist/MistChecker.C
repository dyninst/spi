#include <sys/stat.h>
#include <sys/mman.h>

#include "MistChecker.h"
#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

//------------------------------------------------
// Check process stat
//------------------------------------------------
bool ProcInitChecker::run() {
  print_parent_proc();
  print_current_proc();
  print_user_info();
  print_env();
	return true;
}


void ProcInitChecker::print_user_info() {
  u_.print("  - real user: (name: %s, id: %d)",
	   u_.get_user_name(getuid()).c_str(), getuid());
  u_.print("  - effective user: (name: %s, id: %d)",
	   u_.get_user_name(geteuid()).c_str(), geteuid());
  u_.print("  - real group: (name: %s, id: %d)",
	   u_.get_group_name(getgid()).c_str(), getgid());
  u_.print("  - effective group: (name: %s, id: %d)",
	   u_.get_group_name(getegid()).c_str(), getegid());
}

void ProcInitChecker::print_current_proc() {
  u_.print("* current process");
  print_proc(getpid());
  char buf[256];
  getcwd(buf, 256);
  u_.print("  - working dir: %s", buf);
}

void  ProcInitChecker::print_parent_proc() {
  u_.print("* parent process");
  print_proc(getppid());
}

// XXX: should make it elegant!
void  ProcInitChecker::print_proc(pid_t pid) {
  char path[255];
  u_.print("  - pid: %d", pid);
  snprintf(path, 255, "/proc/%d/cmdline", pid);
  FILE* fp = fopen(path, "r");
  int c;
  // get exe
  string exe;
  while ((c = fgetc(fp)) != '\0') {
    exe += (char)c;
  }
  exe += '\0';
  u_.print("  - exe name: %s", exe.c_str());

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
  for (unsigned i = 0; i < args.size(); i++) {
    cmdline += " ";
    cmdline += args[i].c_str();
  }
  u_.print("  - cmdline arguments: %s", cmdline.c_str());
  fclose(fp);
}

void  ProcInitChecker::print_env() {
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
  u_.print("* environment variables");
  int count = 1;
  for (Envs::iterator i = envs.begin(); i != envs.end(); i++, count++) {
    u_.print("  %d: %s = %s", count, (*i).first.c_str(), (*i).second.c_str());
  }
}

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
