/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "agent/context.h"
#include "agent/ipc/ipc_mgr.h"
#include "agent/patchapi/point.h"
#include "agent/ipc/ipc_workers/ipc_worker_delegate.h"
#include "agent/ipc/ipc_workers/pipe_worker_impl.h"
#include "agent/ipc/ipc_workers/tcp_worker_impl.h"
#include "agent/ipc/ipc_workers/udp_worker_impl.h"
#include "common/utils.h"
#include "common/common.h"
#include "injector/injector.h"
namespace sp {

// Global variables
extern SpContext* g_context;
extern SpParser::ptr g_parser;

//////////////////////////////////////////////////////////////////////

// Instantiate workers for different IPC mechanisms.
SpIpcMgr::SpIpcMgr() {
  pipe_worker_ = new SpPipeWorker;
  worker_set_.insert(pipe_worker_);

  tcp_worker_ = new SpTcpWorker;
  worker_set_.insert(tcp_worker_);

  udp_worker_ = new SpUdpWorker;
  worker_set_.insert(udp_worker_);
}

//////////////////////////////////////////////////////////////////////

// Destroy all workers.
SpIpcMgr::~SpIpcMgr() {
  delete pipe_worker_;
  delete tcp_worker_;
  delete udp_worker_;
}


//////////////////////////////////////////////////////////////////////

// Get parameters from "write" functions.
// Input Param : pt -- the call point from which we get the function
// Output Param: fd_out -- file descriptor, if it is NULL, then skip it
// Output Param: buf_out -- the buffer to write, if NULL, then skip it
// Output Param: c_out -- the character to write, if NULL, then skip it
// Output Param: size_out -- the size of the buffer, if NULL, then skip it
// Output Param: sa_out -- sockaddr for connect(), if NULL, then skip it
void
SpIpcMgr::GetWriteParam(SpPoint* pt,
                        int* fd_out,
                        void** buf_out,
                        char* c_out,
                        size_t* size_out,
                        sockaddr** sa_out) {
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return;

  ArgumentHandle h;
  if (f->name().compare("write") == 0 ||
      f->name().compare("send") == 0) {

    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    void** buf = (void**)sp::PopArgument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = *buf;
    size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = *size;

    sp_debug("IPC GOT WRITE -- %s => fd = %d", f->name().c_str(), *fd_out);
  }

  else if (f->name().compare("writev") == 0) {

    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;

    // const struct iovec *iov
    // sp::PopArgument(pt, &h, sizeof(void*));
    // int* iovcnt = (int*)sp::PopArgument(pt, &h, sizeof(int));
    // fprintf(stderr, "iovcnt: %d\n", *iovcnt);

    sp_debug("IPC GOT WRITE -- %s => fd = %d", f->name().c_str(), *fd_out);
  }

  else if (f->name().compare("connect") == 0) {
    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    sockaddr** sa = (sockaddr**)sp::PopArgument(pt, &h, sizeof(sockaddr*));
    if (sa_out) *sa_out = *sa;

    sp_debug("IPC GOT CONNECT -- %s => fd = %d", f->name().c_str(), *fd_out);
  }

  else if (f->name().compare("fputs") == 0) {
    char** str = (char**)sp::PopArgument(pt, &h, sizeof(char*));
    if (buf_out) *buf_out = (void*)*str;
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("putchar") == 0) {
    int* c = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (c_out) *c_out = *c;
  }

  else if (f->name().compare("fputc") == 0) {
    char* c = (char*)sp::PopArgument(pt, &h, sizeof(char));
    if (c_out) *c_out = *c;
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if (fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("fwrite_unlocked") == 0 ||
           f->name().compare("fwrite") == 0) {
    void** ptr = (void**)sp::PopArgument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = (void*)*ptr;
    size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    size_t* n = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = (*size) * (*n);
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("sendfile64") == 0) {
    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;

    sp_debug("IPC GOT sendfile -- %s => fd = %d", f->name().c_str(), *fd_out);
  }
/* if((*fd_out)!=-1) {
   	fcntl((*fd_out), F_SETOWN,getpid() ==-1);
 }*/ 
}

//////////////////////////////////////////////////////////////////////

// Get parameters from "read" functions.
// Input Param : pt -- the call point from which we get the function
// Output Param: fd_out -- file descriptor, if it is NULL, then skip it
// Output Param: buf_out -- the buffer to write, if NULL, then skip it
// Output Param: size_out -- the size of the buffer, if NULL, then skip it
void
SpIpcMgr::GetReadParam(SpPoint* pt,
                       int* fd_out,
                       void** buf_out,
                       size_t* size_out) {
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return;
  // sp_print("%s", f->name().c_str());
  ArgumentHandle h;
  if (f->name().compare("read") == 0 ||
      f->name().compare("recv") == 0) {
    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    void** buf = (void**)sp::PopArgument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = *buf;
    size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = *size;

    sp_debug("IPC GOT READ -- %s => fd = %d", f->name().c_str(), *fd_out);
  }

  else if (f->name().compare("readv") == 0) {

    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;

    // const struct iovec *iov
    // sp::PopArgument(pt, &h, sizeof(void*));
    // int* iovcnt = (int*)sp::PopArgument(pt, &h, sizeof(int));
    // fprintf(stderr, "iovcnt: %d\n", *iovcnt);

    sp_debug("IPC GOT READ -- %s => fd = %d", f->name().c_str(), *fd_out);
  }

  else if (f->name().compare("fgets") == 0) {
    char** str = (char**)sp::PopArgument(pt, &h, sizeof(char*));
    if (buf_out) *buf_out = (void*)*str;
    int* size = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (size_out) *size_out = *size;
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("fgetc") == 0) {
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("fread_unlocked") == 0 ||
           f->name().compare("fread") == 0) {
    void** ptr = (void**)sp::PopArgument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = (void*)*ptr;
    size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    size_t* n = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = (*size) * (*n);
    FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  else if (f->name().compare("accept") == 0) {
    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;

    sp_debug("IPC GOT ACCEPT -- %s => fd = %d", f->name().c_str(), *fd_out);
  }
/* if((*fd_out)!=-1) {
        fcntl((*fd_out), F_SETOWN,getpid() ==-1);
 }*/

}

//////////////////////////////////////////////////////////////////////

// Get parameters from "close" functions.
// Input Param : pt -- the call point from which we get the function
// Output Param: fd_out -- file descriptor, if it is NULL, then skip it
void
SpIpcMgr::GetCloseParam(SpPoint* pt,
                        int* fd_out) {
  
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return;

  ArgumentHandle h;
  if (f->name().compare("close") == 0) {
    int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    sp_debug("CLOSE -- %s fd = %d", f->name().c_str(), *fd_out);
  }
 /* if((*fd_out)!=-1) {
        fcntl((*fd_out), F_SETOWN,getpid() ==-1);
 }*/
}

//////////////////////////////////////////////////////////////////////
// Get "socket" functions and fcntl to receive SIGURG signals
// Input Param : pt -- the call point from which we get the function
// Output Param: fd_out -- file descriptor, if it is NULL, then skip it
void
SpIpcMgr::fcntlReturnParam(SpPoint* pt) {

  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return;
  int fd;
  ArgumentHandle h;
  if (f->name().compare("socket") == 0) {
    fd = sp::ReturnValue(pt);
    sp_debug("SOCKET -- %s fd = %d", f->name().c_str(), fd);
  }
  if (f->name().compare("accept") == 0) {
    int fd = sp::ReturnValue(pt);
    sp_debug("SOCKET -- %s fd = %d", f->name().c_str(), fd);
  }
 /* if(fd!=-1) {
        fcntl(fd, F_SETOWN,getpid() ==-1);
 }*/
}



// See if the function is a fork
bool
SpIpcMgr::IsFork(const char* f) {
  if (strcmp(f, "fork") == 0) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////

// See if the function is a popen
bool
SpIpcMgr::IsPopen(const char* f) {
  if (strcmp(f, "popen") == 0) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////

// See if current process is allowed to execute the payload code
// This is used in the user-defined payload function.
// Return 1 if it is allowed to execute payload code (for tracing);
// otherwise, 0 is returned.
char SpIpcMgr::CanStartTracing(int fd) {
  return GetWorker(fd)->CanStartTracing(fd);
}

// Get a worker according to the file descriptor.
// Return the worker if the file descriptor is for supported IPC;
// otherwise, return NULL.
SpIpcWorkerDelegate* SpIpcMgr::GetWorker(int fd) {
  // PIPE
  if (IsPipe(fd)) {
    sp_debug("PIPE FD - fd = %d", fd);
    return pipe_worker();
  }
  // TCP
  else if (IsTcp(fd)) {
    sp_debug("TCP FD - fd = %d", fd);
    return tcp_worker();
  }
  // UDP
  else if (IsUdp(fd)) {
    return udp_worker();
  }
  // No IPC
  else {
    return NULL;
  }
}

//////////////////////////////////////////////////////////////////////
// Will be called before user-specified entry-payload function.
bool
SpIpcMgr::BeforeEntry(SpPoint* pt) {
  
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) {
    sp_debug("CALLEE NOT FOUND - in BeforeEntry for call insn %lx",
             pt->block()->last());
    return false;
  }

  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  // Both sender-side and receiver-side
  int fd = -1;
  SpIpcWorkerDelegate* worker = NULL;
  ipc_mgr->GetCloseParam(pt, &fd);
  if (fd != -1 && (worker = ipc_mgr->GetWorker(fd))) {
    //connfd=fd;    
    worker->CloseChannel(fd);
    return true;
  }
  
  // Sender-side
  fd = -1;
  sockaddr* sa = NULL;
  ipc_mgr->GetWriteParam(pt, &fd, NULL, NULL, NULL, &sa);
  if (fd != -1 && (worker = ipc_mgr->GetWorker(fd))) {
    //connfd=fd;
    SpChannel* c = worker->GetChannel(fd, SP_WRITE, sa);
    if (c) {

      // Inject this agent.so to remote process
      // Luckily, the SpInjector implementation will automatically detect
      // whether the agent.so library is already injected. If so, it will
      // not inject the library again.
      if (worker->Inject(c))
      {
      pt->SetChannel(c);
      worker->SetRemoteStartTracing(1, c);
      }
      else
      {
	sp_debug("Not injected in ipc_mgr");
      }
    } else {
      sp_debug("FAILED TO CREATE CHANNEL - for write");
    }

    return true;
  }

  // Receiver-side
  fd = -1;
  worker = NULL;
  ipc_mgr->GetReadParam(pt, &fd, NULL, NULL);
  if (fd != -1 && (worker = ipc_mgr->GetWorker(fd))) {
    //connfd=fd;
    SpChannel* c = worker->GetChannel(fd, SP_READ);

    if (c) {
      pt->SetChannel(c);
    } else {
      sp_debug("FAILED TO CREATE CHANNEL - for read");
    }
  }

  ipc_mgr->HandleExec(pt);
  return true;
}

//////////////////////////////////////////////////////////////////////

// Will be called before user-specified exit-payload function.
bool
SpIpcMgr::BeforeExit(PointHandle* handle) {
  SpFunction* f = handle->GetCallee();
  if (!f) return false;

  // fcntl F_SETOWN socket descriptors returned, so that it can receive OOB packets
  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  //Detect fork for pipe
  if (ipc_mgr->IsFork(f->name().c_str())) {
    long pid = handle->ReturnValue();
    sp_debug("fork: pid[%ld]", pid);
    // Receiver
    if (pid == 0) {
      ipc_mgr->pipe_worker()->SetLocalStartTracing(0);
    } else {
      // change file descriptors for the child
      char error_file[255],output_file[255];
      snprintf(error_file,255,"%s/%s/tmp/spi/spi-error-%d", getenv("SP_DIR"), getenv("PLATFORM"), getpid());
      g_error_fp=fopen(error_file , "a+");
      if (g_error_fp == NULL) {
        std::cerr << "Failed to open file for output erro info: " << strerror(errno) << std::endl;
        std::cerr << "Using stderr instead...\n";
        g_error_fp = stderr;
      }

      snprintf(output_file,255,"%s/%s/tmp/spi/spi-output-%d", getenv("SP_DIR"), getenv("PLATFORM"), getpid());
      g_output_fp=fopen(output_file , "a+");
      if (g_output_fp == NULL) {
        std::cerr << "Failed to open file for output stdout info: " << strerror(errno) << std::endl;
        std::cerr << "Using stderr instead...\n";
        g_output_fp = stderr;
      }

      // Enalbe outputing debug info to /tmp/spi-$PID
      if (getenv("SP_FDEBUG")) {
        char fn[255];
        snprintf(fn, 255, "%s/%s/tmp/spi/spi-%d", getenv("SP_DIR"), getenv("PLATFORM"), getpid());
        g_debug_fp = fopen(fn, "a+");
        if (g_debug_fp == NULL) {
          std::cerr << "Failed to open file for output debug info: " << strerror(errno) << std::endl;
          std::cerr << "Using stderr instead...\n";
          g_debug_fp = stderr;
        }
      }
    }
  }
  // Detect popen for pipe
  else if (ipc_mgr->IsPopen(f->name().c_str())) {
    FILE* fp = (FILE*)handle->ReturnValue();
    int fd = fileno(fp);
    // XXX: magic?? This is a very artificial way to wait for fork done
    sleep(5);
    SpChannel* c = ipc_mgr->pipe_worker()->GetChannel(fd, SP_WRITE);
    ipc_mgr->pipe_worker()->SetRemoteStartTracing(0, c);
  }

  return true;
}

/*
 * In the case of exec family functions, we want the agent library to
 * get propelled into the new program
 * In the case of execve, we have to construct new environment
 * variables so that we can add LD_PRELOAD and etc, and we call execve
 * In other cases, we simply add LD_PRELOAD to the parent process as
 * the environment variables are copied to the new program
 */
void SpIpcMgr::HandleExec(SpPoint* pt) {
  SpFunction* callee = sp::Callee(pt);
  if (callee->name().compare("execve") == 0) {
    sp::ArgumentHandle h;
    char** path = (char**)PopArgument(pt, &h, sizeof(char*));
    char*** argvs = (char***)PopArgument(pt, &h, sizeof(char**));
    char*** envs = (char***)PopArgument(pt, &h, sizeof(char**));

    char** new_envs = (char**)malloc(1024 * sizeof(char*));
    char** ptr = *envs;
    int cur = 0;

    bool ld_preload = false;
    bool ld_library_path = false;
    bool platform = false;
    bool sp_agent_dir = false;
    if (ptr != NULL) {
      while (*ptr != NULL) {
        // if the key is LD_PRELOAD, we append the agent library
        if (strstr(*ptr, "LD_PRELOAD=") == *ptr) {
          new_envs[cur] = (char*)malloc(1024 * sizeof(char));
          if (sp::g_context) {
            snprintf(new_envs[cur], 1024, "%s:%s/%s", *ptr,
                     getenv("SP_AGENT_DIR"), sp::g_context->getAgentName().c_str());
          } else {
            snprintf(new_envs[cur], 1024, "%s:%s/%s", *ptr,
                     getenv("SP_AGENT_DIR"), "libmyagent.so");
          }
          ld_preload = true;
        } else if (strstr(*ptr, "LD_LIBRARY_PATH=") == *ptr) {
          // if the key is LD_LIBRARY_PATH, we append the agent dir
          new_envs[cur] = (char*)malloc(1024 * sizeof(char));
          snprintf(new_envs[cur], 1024, "%s:%s:%s/%s", *ptr,
                   getenv("SP_AGENT_DIR"), getenv("SP_DIR"),
                   getenv("PLATFORM"));
          ld_library_path = true;
        } else if (strstr(*ptr, "PLATFORM=") == *ptr) {
          platform = true;
        } else if (strstr(*ptr, "SP_AGENT_DIR=") == *ptr) {
          sp_agent_dir = true;
        } else {
          new_envs[cur] = *ptr;
          sp_print("%s", new_envs[cur]);
        }
        cur++;
        ptr++;
      }
    }

    if (!ld_preload) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      if (sp::g_context) {
        snprintf(new_envs[cur], 1024, "LD_PRELOAD=%s/%s",
                  getenv("SP_AGENT_DIR"), sp::g_context->getAgentName().c_str());
      } else {
        snprintf(new_envs[cur], 1024, "LD_PRELOAD=%s/%s",
                  getenv("SP_AGENT_DIR"), "libmyagent.so");
      }
    }
    if (!ld_library_path) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      snprintf(new_envs[cur], 1024, "LD_LIBRARY_PATH=%s:%s/%s",
                getenv("SP_AGENT_DIR"), getenv("SP_DIR"),
                getenv("PLATFORM"));
      ld_library_path = true;
    }
    if (!platform) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      snprintf(new_envs[cur], 1024, "PLATFORM=%s", getenv("PLATFORM"));
      cur++;
    }
    if (!sp_agent_dir) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      snprintf(new_envs[cur], 1024, "SP_AGENT_DIR=%s", getenv("SP_AGENT_DIR"));
      cur++;
    }
    if (getenv("SP_IPC")) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      snprintf(new_envs[cur], 1024, "SP_IPC=1");
      cur++;
    }

    if (getenv("SP_FDEBUG")) {
      new_envs[cur] = (char*)malloc(1024 * sizeof(char));
      snprintf(new_envs[cur], 1024, "SP_FDEBUG=1");
      cur++;
    }
    new_envs[cur] = NULL;
    cur++;

    execve(*path, *argvs, new_envs);
    system("touch /tmp/fail_execve");
  } else if (callee->name().compare("execl") == 0 ||
             callee->name().compare("execlp") == 0 ||
             callee->name().compare("execle") == 0 ||
             callee->name().compare("execv") == 0 ||
             callee->name().compare("execvp") == 0) {
    std::string agent_name = g_parser->agent_name();
    if (char* ld_preload_str = getenv("LD_PRELOAD")) {
      std::string libs{ld_preload_str};
      size_t start = libs.find(agent_name);
      if (start == std::string::npos) {
        // agent lib is not in the LD_PRELOAD, append it
        libs += ":";
        libs += getenv("SP_AGENT_DIR");
        libs += "/";
        libs += agent_name;
        setenv("LD_PRELOAD", libs.c_str(), 1);
      }
    } else {
      // no LD_PRELOAD environment variable yet,
      // we will set it to our agent lib
      std::string tmp = getenv("SP_AGENT_DIR");
      tmp += "/";
      tmp += agent_name;
      setenv("LD_PRELOAD", tmp.c_str(), 1);
    }
  }
}

} // Namespace sp
