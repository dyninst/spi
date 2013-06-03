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

#include "agent/context.h"
#include "agent/ipc/ipc_mgr.h"
#include "agent/patchapi/point.h"
#include "agent/ipc/ipc_workers/ipc_worker_delegate.h"
#include "agent/ipc/ipc_workers/pipe_worker_impl.h"
#include "agent/ipc/ipc_workers/tcp_worker_impl.h"
#include "agent/ipc/ipc_workers/udp_worker_impl.h"
#include "common/utils.h"
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
}

//////////////////////////////////////////////////////////////////////

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
    
    worker->CloseChannel(fd);
    return true;
  }
  
  // Sender-side
  fd = -1;
  sockaddr* sa = NULL;
  ipc_mgr->GetWriteParam(pt, &fd, NULL, NULL, NULL, &sa);
  if (fd != -1 && (worker = ipc_mgr->GetWorker(fd))) {
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
    SpChannel* c = worker->GetChannel(fd, SP_READ);

    if (c) {
      pt->SetChannel(c);
    } else {
      sp_debug("FAILED TO CREATE CHANNEL - for read");
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

// Will be called before user-specified exit-payload function.
bool
SpIpcMgr::BeforeExit(SpPoint* pt) {

  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return false;

  // Detect fork for pipe
  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();
  if (ipc_mgr->IsFork(f->name().c_str())) {
    long pid = sp::ReturnValue(pt);
    // Receiver
    if (pid == 0) {
      ipc_mgr->pipe_worker()->SetLocalStartTracing(0);
    }
  }
  // Detect popen for pipe
  else if (ipc_mgr->IsPopen(f->name().c_str())) {
    FILE* fp = (FILE*)sp::ReturnValue(pt);
    int fd = fileno(fp);
    // XXX: magic?? This is a very artificial way to wait for fork done
    sleep(5);
    SpChannel* c = ipc_mgr->pipe_worker()->GetChannel(fd, SP_WRITE);
    ipc_mgr->pipe_worker()->SetRemoteStartTracing(0, c);
  }

  return true;
}

} // Namespace sp
