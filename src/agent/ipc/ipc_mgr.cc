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

  // Instantiate workers for different IPC mechanisms.
  SpIpcMgr::SpIpcMgr() {
    pipe_worker_ = new SpPipeWorker;
    worker_set_.insert(pipe_worker_);

    tcp_worker_ = new SpTcpWorker;
    worker_set_.insert(tcp_worker_);

    udp_worker_ = new SpUdpWorker;
    worker_set_.insert(udp_worker_);
  }

  // Destroy all workers.
  SpIpcMgr::~SpIpcMgr() {
    delete pipe_worker_;
    delete tcp_worker_;
    delete udp_worker_;
  }


  // Get parameters from "write" functions.
  // Input Param : pt -- the call point from which we get the function
  // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
  // Output Param: buf_out -- the buffer to write, if NULL, then skip it
  // Output Param: c_out -- the character to write, if NULL, then skip it
  // Output Param: size_out -- the size of the buffer, if NULL, then skip it
  // Output Param: sa_out -- sockaddr for connect(), if NULL, then skip it
  void
  SpIpcMgr::get_write_param(SpPoint* pt, int* fd_out, void** buf_out,
                            char* c_out, size_t* size_out, sockaddr** sa_out) {
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

      /*
        if (sa_out && is_tcp(*fd)) {
        sp_debug("GET REM_SA - for fd %d at a %s ()", *fd, f->name().c_str());
        sockaddr_in rem_sa;
        memset(&rem_sa, 0, sizeof(sockaddr_in));
        socklen_t rem_len = sizeof(sockaddr_in);
        if (getpeername(*fd, (sockaddr*)&rem_sa, &rem_len) == -1) {
        perror("getsockname");
        }
        sp_debug("REMOTE IP - %s", inet_ntoa(rem_sa.sin_addr));
        sp_debug("REMOTE PORT - %d", htons(rem_sa.sin_port));
        }
      */
    }

    if (f->name().compare("connect") == 0) {
      int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
      sockaddr** sa = (sockaddr**)sp::PopArgument(pt, &h, sizeof(sockaddr*));
      // sockaddr* sa_tmp = (sockaddr*)malloc(sizeof(sockaddr));
      // memcpy(sa_tmp, *sa, sizeof(sockaddr));
      if (sa_out) *sa_out = *sa;
    }

    if (f->name().compare("fputs") == 0) {
      char** str = (char**)sp::PopArgument(pt, &h, sizeof(char*));
      if (buf_out) *buf_out = (void*)*str;
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if(fd_out) *fd_out = fileno(*fp);
    }

    if (f->name().compare("fputc") == 0) {
      char* c = (char*)sp::PopArgument(pt, &h, sizeof(char));
      if (c_out) *c_out = *c;
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if (fd_out) *fd_out = fileno(*fp);
    }

    if (f->name().compare("fwrite_unlocked") == 0 ||
        f->name().compare("fwrite") == 0) {
      void** ptr = (void**)sp::PopArgument(pt, &h, sizeof(void*));
      if (buf_out) *buf_out = (void*)*ptr;
      size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
      size_t* n = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
      if (size_out) *size_out = (*size) * (*n);
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if(fd_out) *fd_out = fileno(*fp);
    }
  }

  // Get parameters from "read" functions.
  // Input Param : pt -- the call point from which we get the function
  // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
  // Output Param: buf_out -- the buffer to write, if NULL, then skip it
  // Output Param: size_out -- the size of the buffer, if NULL, then skip it
  void
  SpIpcMgr::get_read_param(SpPoint* pt, int* fd_out, void** buf_out,
                           size_t* size_out) {
    ph::PatchFunction* f = sp::Callee(pt);
    if (!f) return;

    ArgumentHandle h;
    if (f->name().compare("read") == 0 ||
        f->name().compare("recv") == 0) {
      int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
      void** buf = (void**)sp::PopArgument(pt, &h, sizeof(void*));
      if (buf_out) *buf_out = *buf;
      size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
      if (size_out) *size_out = *size;
    }

    if (f->name().compare("fgets") == 0) {
      char** str = (char**)sp::PopArgument(pt, &h, sizeof(char*));
      if (buf_out) *buf_out = (void*)*str;
      int* size = (int*)sp::PopArgument(pt, &h, sizeof(int));
      if (size_out) *size_out = *size;
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if(fd_out) *fd_out = fileno(*fp);
    }

    if (f->name().compare("fgetc") == 0) {
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if(fd_out) *fd_out = fileno(*fp);
    }

    if (f->name().compare("fread_unlocked") == 0 ||
        f->name().compare("fread") == 0) {
      void** ptr = (void**)sp::PopArgument(pt, &h, sizeof(void*));
      if (buf_out) *buf_out = (void*)*ptr;
      size_t* size = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
      size_t* n = (size_t*)sp::PopArgument(pt, &h, sizeof(size_t));
      if (size_out) *size_out = (*size) * (*n);
      FILE** fp = (FILE**)sp::PopArgument(pt, &h, sizeof(FILE*));
      if(fd_out) *fd_out = fileno(*fp);
    }

    if (f->name().compare("accept") == 0) {
      int* fd = (int*)sp::PopArgument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
    }

  }


  // See if the function is a fork
  bool
  SpIpcMgr::is_fork(const char* f) {
    if (strcmp(f, "fork") == 0) return true;
    return false;
  }

  // See if the function is a popen
  bool
  SpIpcMgr::is_popen(const char* f) {
    if (strcmp(f, "popen") == 0) return true;
    return false;
  }

  // See if current process is allowed to execute the payload code
  // This is used in the user-defined payload function.
  // Return 1 if it is allowed to execute payload code (for tracing);
  // otherwise, 0 is returned.
  char SpIpcMgr::start_tracing(int fd) {
    sp_print("ipcmgr:start_tracing");
    for (WorkerSet::iterator wi = worker_set_.begin();
         wi != worker_set_.end(); wi++) {
      if ((*wi)->start_tracing(fd)) return 1;
    }
    return 0;
  }

  // Get a worker according to the file descriptor.
  // Return the worker if the file descriptor is for supported IPC;
  // otherwise, return NULL.
  SpIpcWorkerDelegate* SpIpcMgr::get_worker(int fd) {
    // PIPE
    if (IsPipe(fd)) {
      return pipe_worker();
    }
    // TCP
    else if (IsTcp(fd)) {
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

  // Will be called before user-specified entry-payload function.
  bool
  SpIpcMgr::BeforeEntry(SpPoint* pt) {
    ph::PatchFunction* f = sp::Callee(pt);
    if (!f) return false;

    sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

    // Sender-side
    // Detect initiation of communication
    int fd = -1;
    sockaddr* sa = NULL;
    ipc_mgr->get_write_param(pt, &fd, NULL, NULL, NULL, &sa);
    if (fd != -1) {
      SpIpcWorkerDelegate* worker = ipc_mgr->get_worker(fd);
      if (!worker) return false;
      // Enable tracing for current process
      worker->set_start_tracing(1);

      SpChannel* c = worker->get_channel(fd, SP_WRITE, sa);
      if (c) {
        // Inject this agent.so to remote process
        // Luckily, the SpInjector implementation will automatically detect
        // whether the agent.so library is already injected. If so, it will
        // not inject the library again.
        // if (c->remote_pid != -1) worker->inject(c);
        worker->inject(c);

        // Enable tracing for remote process
        if (Callee(pt)->name().compare("connect") != 0)
          worker->set_start_tracing(1, c);
        pt->SetChannel(c);
      }
      return true;
    }

    // Receiver-side
    fd = -1;
    ipc_mgr->get_read_param(pt, &fd, NULL, NULL);
    if (fd != -1) {
      SpIpcWorkerDelegate* worker = ipc_mgr->get_worker(fd);
      if (!worker) return false;
      SpChannel* c = worker->get_channel(fd, SP_READ);
      if (c) {
        pt->SetChannel(c);
      }
    }
    return true;
  }

  // Will be called before user-specified exit-payload function.
  bool
  SpIpcMgr::BeforeExit(SpPoint* pt) {
    ph::PatchFunction* f = sp::Callee(pt);
    if (!f) return false;

    // Detect fork for pipe
    sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();
    if (ipc_mgr->is_fork(f->name().c_str())) {
      long pid = sp::ReturnValue(pt);
      // Receiver
      if (pid == 0) {
        ipc_mgr->pipe_worker()->set_start_tracing(0);
      }
    }
    // Detect popen for pipe
    else if (ipc_mgr->is_popen(f->name().c_str())) {
      FILE* fp = (FILE*)sp::ReturnValue(pt);
      int fd = fileno(fp);
      // XXX: magic?? This is a very artificial way to wait for fork done
      sleep(2);
      SpChannel* c = ipc_mgr->pipe_worker()->get_channel(fd, SP_WRITE);
      ipc_mgr->pipe_worker()->set_start_tracing(0, c);
    }
    // Detect connect for tcp
    else {
    }
    return true;
  }

} // Namespace sp
