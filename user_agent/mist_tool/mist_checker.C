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
  PrintParentProc();
  PrintUserInfo();
  // PrintEnv();

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
  char entry[1024];

  // Get pid
  snprintf(entry, 1024, "<pid>%d</pid>", pid);
  u_.WriteHeader(entry);

  // Get exe
  char path[255];
  snprintf(path, 255, "/proc/%d/cmdline", pid);
  FILE* fp = fopen(path, "r");

  string exe;
  int c;
  while ((c = fgetc(fp)) != '\0' && c != -1) {
    // sp_print("%c -- %d", (char)c, c);
    exe += (char)c;
  }
  exe += '\0';

  snprintf(entry, 1024, "<exe_name>%s</exe_name>", exe.c_str());
  u_.WriteHeader(entry);

#if 0
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
#endif
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
  u_.WriteString(-10,
                 "<traces></traces></process>");
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
  if (callee->name().compare("connect") == 0) {
    ArgumentHandle h;
    PopArgument(pt, &h, sizeof(int));
    sockaddr** addr = (sockaddr**)PopArgument(pt, &h,
                                              sizeof(sockaddr*));
    char host[256];
    char service[256];
    if (sp::GetAddress((sockaddr_storage*)*addr, host, 256, service, 256)) {
      char buf[1024];
      snprintf(buf, 1024,
               "<trace type=\"connect to\" time=\"%lu\">",
               u_.GetUsec());
      u_.WriteTrace(buf);
      snprintf(buf, 1024,
               "<host>%s</host><port>%s</port>",
               host, service);
      u_.WriteTrace(buf);
      u_.WriteTrace("</trace>");
    } 
  } else if (callee->name().compare("accept") == 0) {
    ArgumentHandle h;
    int* fd = (int*)PopArgument(pt, &h, sizeof(int));
    sockaddr_storage addr;
    if (GetLocalAddress(*fd, &addr)) {
      char host[256];
      char service[256];
      if (sp::GetAddress((sockaddr_storage*)&addr, host, 256, service, 256)) {
        char buf[1024];
        snprintf(buf, 1024,
                 "<trace type=\"accept from\" time=\"%lu\">",
                 u_.GetUsec());
        u_.WriteTrace(buf);
        snprintf(buf, 1024,
                 "<host>%s</host><port>%s</port>",
                 host, service);
        u_.WriteTrace(buf);
        u_.WriteTrace("</trace>");
      }
    }
  }


  return true;
}

////////////////////////////////////////////////////////////////////// 
bool
IpcChecker::post_check(SpPoint* pt,
                       SpFunction* callee) {
  /*
  if (callee->name().compare("accept") == 0) {
    int fd = ReturnValue(pt);
    sockaddr_storage addr;
    if (GetRemoteAddress(fd, &addr)) {
      char host[256];
      char service[256];
      if (sp::GetAddress((sockaddr_storage*)&addr, host, 256, service, 256)) {
        char buf[1024];
        snprintf(buf, 1024,
                 "<trace type=\"accept from\" time=\"%lu\">",
                 u_.GetUsec());
        u_.WriteTrace(buf);
        snprintf(buf, 1024,
                 "<host>%s</host><port>%s</port>",
                 host, service);
        u_.WriteTrace(buf);
        u_.WriteTrace("</trace>");
      }
    }
  }
  */
	return true;
}

// ------------------------------------------------------------------- 
// Fork
// -------------------------------------------------------------------
ForkChecker::ForkChecker(Mist* mist) : mist_(mist) {
}

bool ForkChecker::check(SpPoint* pt, SpFunction* callee) {
  if (callee->name().compare("execve") == 0) {
    // Output trace
    ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**));

    // Modify environment
    // /mnt/hgfs/shared/spi/user_agent/mist_tool/x86_64-unknown-linux2.4/libmyagent.so
    char cmd[1024];
    sprintf(cmd, "echo \"%s\" > /tmp/%d.txt", *path, getpid());
    system(cmd);
    
    char buf[102400];
    char trace_file[255];
    snprintf(trace_file, 255, "/tmp/%d-trace.xml", getpid());
    snprintf(buf, 102400,
             "<trace type=\"%s\" time=\"%lu\">%s",
             callee->name().c_str(), u_.GetUsec(), *path);
    char** ptr = *argvs;
    while (*ptr != NULL) {
      strcat(buf, *ptr);
      ptr++;
    }
    ptr = *envs;
    char **new_envs = (char**)malloc(1024*sizeof(char*));
    int cur = 0;
    while (*ptr != NULL) {
      new_envs[cur++] = *ptr;
      strcat(buf, *ptr);
      ptr++;
    }
    /*
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "LD_PRELOAD=/home/wenbin/devel/spi/user_agent/mist_tool/x86_64-unknown-linux2.4/libmyagent.so");
    cur++;
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "LD_LIBRARY_PATH=/home/wenbin/soft/lib:/home/wenbin/devel/dyninst/x86_64-unknown-linux2.4/lib:/home/wenbin/devel/spi/x86_64-unknown-linux2.4/test_agent:/home/wenbin/devel/spi/x86_64-unknown-linux2.4");
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "PLATFORM=x86_64-unknown-linux2.4");
    cur++;
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "SP_TEST_RELOCINSN=1");
    cur++;
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "SP_AGENT_DIR=/home/wenbin/devel/spi/user_agent/mist_tool/x86_64-unknown-linux2.4");
    cur++;
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
    cur++;
    new_envs[cur] = (char*)malloc(2048);
    strcpy(new_envs[cur], "SP_LSOF=/usr/bin/lsof");
    cur++;

    cur = 0;
    ptr = new_envs;
    while (*ptr != NULL) {
      strcat(buf, *ptr);
      ptr++;
    }
    */
    struct stat s;

    // XXX: why the last trace record is not flushed to disk??
    /*
    if (stat(trace_file, &s) != 0) {
      mist_->fork_init_run();
    }
    */
    if (stat(trace_file, &s) == 0) {
      snprintf(cmd, 1024, "cp /tmp/%d-trace.xml /tmp/%d-exe.xml",
               getpid(), getpid());
      system(cmd);
      char fn[1024];
      sprintf(fn, "/tmp/%d-exe.xml", getpid());
      //      FILE* fp = fopen(fn, "r+");
      FILE* fp = fopen(fn, "a");
      fseek(fp, -19, SEEK_END);
      fprintf(fp, "%s</trace></traces></process>", buf);
      fclose(fp);
    }
    
    // execve(*path, *argvs, new_envs);
    // system("touch /tmp/fail_execve");
    // Setup LD_PRELOAD for job
  }

	return true;
}

bool ForkChecker::post_check(SpPoint* pt, SpFunction* callee) {
  if (callee->name().compare("fork") == 0) {
    long ret = ReturnValue(pt);
    if (ret == 0) {
      mist_->fork_init_run();
    } else {
      char buf[1024];
      snprintf(buf, 1024,
               "<trace type=\"fork\" time=\"%lu\">%lu",
               u_.GetUsec(), ret);
      u_.WriteTrace(buf);
      u_.WriteTrace("</trace>");
    }
  }
  
  return true;
}

// ------------------------------------------------------------------- 
// Clone
// -------------------------------------------------------------------
CloneChecker::CloneChecker(Mist* mist) : mist_(mist) {
}

bool CloneChecker::check(SpPoint* pt, SpFunction* callee) {
  if (callee->name().compare("clone") == 0) {
    // Get callback function instance, record it
    system("touch /tmp/clone_laaaa");
  }

  // If callback function is not NULL, and is callee
  // Start new trace file
  // Run init_run
  
	return true;
}

bool CloneChecker::post_check(SpPoint* pt,
                              SpFunction* callee) {

  if (callee->name().compare("clone") == 0) {
    long ret = ReturnValue(pt);
    if (ret > 0) {
      char buf[1024];
      snprintf(buf, 1024,
               "<trace type=\"clone\" time=\"%lu\">%lu",
               u_.GetUsec(), ret);
      u_.WriteTrace(buf);
      u_.WriteTrace("</trace>");
    } else if (ret == 0){
      // mist_->fork_init_run();
    }
  }

  return true;
}

// check uid/gid changes
// Check the changes of uid/gid
bool ChangeIdChecker::check(SpPoint* pt,
                            SpFunction* callee) {
  if (callee->name().compare("seteuid") == 0 ||
      callee->name().compare("setuid") == 0 ) {
    ArgumentHandle h;
    uid_t* uid = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
    char buf[1024];
    snprintf(buf, 1024,
             "<trace type=\"%s\" time=\"%lu\">%d",
             callee->name().c_str(), u_.GetUsec(), *uid);
    u_.WriteTrace(buf);
    u_.WriteTrace("</trace>");
  } else if (callee->name().compare("seteuid") == 0) {
    ArgumentHandle h;
    PopArgument(pt, &h, sizeof(uid_t));
    uid_t* uid = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
    char buf[1024];
    snprintf(buf, 1024,
             "<trace type=\"%s\" time=\"%lu\">%d",
             callee->name().c_str(), u_.GetUsec(), *uid);
    u_.WriteTrace(buf);
    u_.WriteTrace("</trace>");
  }

	return true;
}

}

// set_user_euid
