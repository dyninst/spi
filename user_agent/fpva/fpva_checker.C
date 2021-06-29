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
  if (callee->name().compare("execve") == 0) {
    sp_print("shal: In execve");
    // Output trace
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**));

  } else if (callee->name().compare("execl") == 0 ||
             callee->name().compare("execlp") == 0 ||
             callee->name().compare("execle") == 0 ||
             callee->name().compare("execv") == 0 ||
             callee->name().compare("execvp") == 0) {
    // Output trace
    sp_print("shal: in other than execve");
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));

    // TODO: Flush the file
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
    pid_t ret = (pid_t) pHandle->GetReturnValue();
    if (ret == 0) {
      mgr_->NewDoc();
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

  return true;
}

}  // namespace mist
