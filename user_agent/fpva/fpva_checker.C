#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SpInc.h"
#include "fpva.h"
#include "fpva_checker.h"

// Used by ForkChecker to set execl's environment vars
extern char** environ;

namespace sp {
extern SpContext* g_context;
}

namespace fpva {

// basic constructor
OneTimeChecker::OneTimeChecker(TraceMgr* mgr) : mgr_(mgr) {}

FpvaChecker::FpvaChecker(TraceMgr* mgr) : mgr_(mgr) {}

//------------------------------------------------
// Check process stat
//------------------------------------------------

//////////////////////////////////////////////////

bool ProcInitChecker::Run() {
  sp_print("ProcInitChecker for pid=%d\n", getpid());
  mgr_->PrintCurrentProc();
  mgr_->PrintParentProc();
  mgr_->PrintUserInfo();
  return true;
}

// -------------------------------------------------------------------
// Ipc Checker
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

bool IpcChecker::PreCheck(sp::SpPoint* pt, sp::SpFunction* callee) {
  if (callee->name().compare("connect") == 0) {
    sp::ArgumentHandle h;
    PopArgument(pt, &h, sizeof(int));
    sockaddr** addr = (sockaddr**)PopArgument(pt, &h, sizeof(sockaddr*));
    char host[256];
    char service[256];
    sp_print("FPVA got connect");
    if (sp::GetAddress((sockaddr_storage*)*addr, host, 256, service, 256)) {
      sp_print("INSTRUMENTATION(PID=%d): connect() to %s:%s ...\n", getpid(),
               host, service);
      CallTrace newTrace;
      newTrace.functionName = "connect";
      newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("host", host));
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("port", service));
      mgr_->AddTrace(&newTrace);
    }
  } else if (callee->name().compare("accept") == 0) {
    sp::ArgumentHandle h;
    int* fd = (int*)PopArgument(pt, &h, sizeof(int));
    sockaddr_storage addr;
    if (sp::GetLocalAddress(*fd, &addr)) {
      char host[256];
      char service[256];
      if (sp::GetAddress((sockaddr_storage*)&addr, host, 256, service, 256)) {
        CallTrace newTrace;
        newTrace.functionName = "accept";
        newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
        newTrace.parameters.insert(
            std::pair<std::string, std::string>("host", host));
        newTrace.parameters.insert(
            std::pair<std::string, std::string>("port", service));
        mgr_->AddTrace(&newTrace);
      }
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////
bool IpcChecker::PostCheck(sp::PointCallHandle*) { return true; }

/*
 * In the case of exec family functions, we want the agent library to
 * get propelled into the new program
 * In the case of execve, we have to construct new environment
 * variables so that we can add LD_PRELOAD and etc, and we call execve
 * In other cases, we simply add LD_PRELOAD to the parent process as
 * the environment variables are copied to the new program
 */
bool ForkChecker::PreCheck(sp::SpPoint* pt, sp::SpFunction* callee) {

  if (callee->name().compare("execve") == 0 ||
      callee->name().compare("execvpe") == 0) {
    sp_print("shal: In exec with envs");
    // Output trace
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**)); 
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(std::pair<std::string, std::string>("path", std::string(*path)));
    char** ptr = *argvs;
    if (ptr != NULL)
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("argvs", std::string(*ptr)));
        ptr++;
    }
    ptr = *envs;
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("envs", std::string(*ptr)));
        ptr++;
    }
    mgr_->AddTrace(&newTrace);
  } else if (callee->name().compare("fexecve") == 0 ) {
    sp_print("shal: In fexecve");
    // Output trace
    sp::ArgumentHandle h;
    int* file = (int*)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**)); 
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(std::pair<std::string, std::string>("path", std::to_string(*file)));
    char** ptr = *argvs;
    if (ptr != NULL)
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("argvs", std::string(*ptr)));
        ptr++;
    }
    ptr = *envs;
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("envs", std::string(*ptr)));
        ptr++;
    }
    mgr_->AddTrace(&newTrace);
  } else if (callee->name().compare("execveat") == 0) {
    sp_print("shal: In exec with envs");
    // Output trace
    sp::ArgumentHandle h;
    int* dirfd = (int*)PopArgument(pt, &h, sizeof(int));
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**)); 
    int* flags = (int*)PopArgument(pt, &h, sizeof(int));

    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(std::pair<std::string, std::string>("directory_fd", std::to_string(*dirfd)));
    newTrace.parameters.insert(std::pair<std::string, std::string>("path", std::string(*path)));
    char** ptr = *argvs;
    if (ptr != NULL)
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("argvs", std::string(*ptr)));
        ptr++;
    }
    ptr = *envs;
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("envs", std::string(*ptr)));
        ptr++;
    }
    newTrace.parameters.insert(std::pair<std::string, std::string>("flags", std::to_string(*flags)));
 
    mgr_->AddTrace(&newTrace);
  } else if (callee->name().compare("execle") == 0) {
    sp_print("shal: In execle");
    // Output trace
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char** argv = (char**)PopArgument(pt, &h, sizeof(char*));
    
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(std::pair<std::string, std::string>("path", std::string(*path)));
 
    while (argv != NULL){
        argv = (char**)PopArgument(pt, &h, sizeof(char*));
        newTrace.parameters.insert(std::pair<std::string, std::string>("argvs", std::string(*argv)));    
    }
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**)); 
    char** ptr = *envs;
    while (*ptr != NULL) {
        newTrace.parameters.insert(std::pair<std::string, std::string>("envs", std::string(*ptr)));
        ptr++;
    } 
    mgr_->AddTrace(&newTrace);

  } else if (callee->name().compare("execl") == 0 ||
             callee->name().compare("execlp") == 0 ||
             callee->name().compare("execv") == 0 ||
             callee->name().compare("execvp") == 0 ) {
    // Output trace
    sp_print("shal: in other exec");
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));

    // TODO: Flush the file
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
	std::pair<std::string, std::string>("path", std::string(*path)));
    mgr_->AddTrace(&newTrace);
  }
  return true;
}

/**
 * check the return value of fork
 * If we are in the parent process, add a new trace
 * If we are in the child process, update the xml doc because we don't want
 * two duplicate xml trace files
 */
bool ForkChecker::PostCheck(sp::PointCallHandle* pHandle) {
  sp::SpFunction* callee = pHandle->GetCallee();
  if (callee->name().compare("fork") == 0) {
    pid_t ret = (pid_t)pHandle->GetReturnValue();
    if (ret == 0) {
      mgr_->NewDoc();
      mgr_->PrintCurrentProc();
      mgr_->PrintParentProc();
      mgr_->PrintUserInfo();
    } else {
      sp_print("INSTRUMENTATION(pid=%d): fork() child pid = %d...\n", getpid(),
               ret);
      CallTrace newTrace;
      newTrace.functionName = "fork";
      newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("pid", std::to_string(ret)));
      mgr_->AddTrace(&newTrace);
    }
  }
  else if (callee->name().compare("clone") == 0||
           callee->name().compare("clone2") == 0||
           callee->name().compare("clone3") == 0) {
    pid_t ret = (pid_t)pHandle->GetReturnValue();
    if (ret == 0) {
      mgr_->NewDoc();
      mgr_->PrintCurrentProc();
      mgr_->PrintParentProc();
      mgr_->PrintUserInfo();
    } else {
      sp_print("INSTRUMENTATION(pid=%d): clone() child pid = %d/...\n", getpid(), ret);
      CallTrace newTrace;
      newTrace.functionName = "clone";
      newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("pid", std::to_string(ret)));
      mgr_->AddTrace(&newTrace);
    }
  }
  return true;
}

bool FileChecker::PreCheck(sp::SpPoint* pt, sp::SpFunction* callee) {
  if (callee->name() == "open") {
    sp::ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::string(*fname)));
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "chmod") {
    sp::ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    mode_t* mode = (mode_t*)PopArgument(pt, &h, sizeof(mode_t));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::string(*fname)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("mode", std::to_string(*mode)));
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "fopen") {
    sp::ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    mode_t* mode = (mode_t*)PopArgument(pt, &h, sizeof(mode_t));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::string(*fname)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("mode", std::to_string(*mode)));
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "fdopen") {
    sp::ArgumentHandle h;
    int* fd = (int*)PopArgument(pt, &h, sizeof(int));
    mode_t* mode = (mode_t*)PopArgument(pt, &h, sizeof(mode_t));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::to_string(*fd)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("mode", std::to_string(*mode)));
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "freopen") {
    sp::ArgumentHandle h;
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    mode_t* mode = (mode_t*)PopArgument(pt, &h, sizeof(mode_t));
    //FILE* stream = (FILE*)PopArgument(pt, &h, sizeof(FILE));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::string(*fname)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("mode", std::to_string(*mode)));
    //newTrace.parameters.insert(
    //    std::pair<std::string, std::string>("stream", std::to_string(*stream)));
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "openat") {
    sp::ArgumentHandle h;
    int* dirfd = (int*)PopArgument(pt, &h, sizeof(int)); 
    char** fname = (char**)PopArgument(pt, &h, sizeof(void*));
    int* flags = (int*)PopArgument(pt, &h, sizeof(int)); 
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("directory_fd", std::to_string(*dirfd)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("file", std::string(*fname)));
     newTrace.parameters.insert(
        std::pair<std::string, std::string>("flags", std::to_string(*flags))); 
    mgr_->AddTrace(&newTrace);
    return true;
  } else if (callee->name() == "chown" ||
             callee->name() == "fchown" ||
             callee->name() == "lchown" ||
             callee->name() == "fchownat" ) {
    sp::ArgumentHandle h;

    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());

    if (callee->name() == "fchownat"){ 
      int* dirfd = (int*)PopArgument(pt, &h, sizeof(int));
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("directory_fd", std::to_string(*dirfd)));
    }
    if (callee->name() == "fchown"){
      int* fd = (int*)PopArgument(pt, &h, sizeof(int));
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("file", std::to_string(*fd)));
    } 
    else {
      char** pathname = (char**)PopArgument(pt, &h, sizeof(char*));
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("file", std::string(*pathname))); 
    }

    uid_t* owner = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("owner", std::to_string(*owner)));
 
    gid_t* group = (gid_t*)PopArgument(pt, &h, sizeof(uid_t));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("group", std::to_string(*group)));
 
    if (callee->name() == "fchownat"){
      int* flags = (int*)PopArgument(pt, &h, sizeof(int));
      newTrace.parameters.insert(
          std::pair<std::string, std::string>("flags", std::to_string(*flags))); 
    }

    mgr_->AddTrace(&newTrace);
    return true;
  } 
  return false;
}

bool FileChecker::PostCheck(sp::PointCallHandle*) { return true; }

bool PrivilegeChecker::PreCheck(sp::SpPoint* pt, sp::SpFunction* callee) {
  if (callee->name() == "seteuid" || callee->name() == "setuid") {
    sp::ArgumentHandle h;
    uid_t* uid = (uid_t*)PopArgument(pt, &h, sizeof(uid_t));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(std::pair<std::string, std::string>(
        "name", FpvaUtils::GetUserName(*uid)));
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("uid", std::to_string(*uid)));
    mgr_->AddTrace(&newTrace);
    return true;
  }
  else if (callee->name() == "chroot" || callee->name() == "chdir") {
    sp::ArgumentHandle h;
    char** pathname = (char**)PopArgument(pt, &h, sizeof(char*));
    CallTrace newTrace;
    newTrace.functionName = callee->name();
    newTrace.timeStamp = std::to_string(FpvaUtils::GetUsec());
    newTrace.parameters.insert(
        std::pair<std::string, std::string>("directory", std::string(*pathname)));
    mgr_->AddTrace(&newTrace);
    
    return true;
  }
  return false;
}

bool PrivilegeChecker::PostCheck(sp::PointCallHandle*) { return true; }

}  // namespace fpva
