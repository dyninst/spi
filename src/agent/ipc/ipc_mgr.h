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

#ifndef _SPIPCMGR_H_
#define _SPIPCMGR_H_

#include "common/common.h"

#include "agent/ipc/channel.h"
#include "agent/ipc/ipc_workers/ipc_worker_delegate.h"
#include "common/utils.h"


namespace sp {

  // Forword declarations
  class SpPoint;
  class SpIpcWorker;
  class SpPipeWorker;
  class SpUdpWorker;
  class SpTcpWorker;


  // IPC Manager, which is an all-in-one manager for inter-process support.

  class SpIpcMgr {
 public:
    SpIpcMgr();
    ~SpIpcMgr();

    // Get channel from fd
    // If channel doesn't exist, construct one
    // Return NULL if failed to create one channel
    SpChannel* GetChannel(int fd,
                          ChannelRW rw);

    // Get a worker according to the file descriptor.
    // Return the worker if the file descriptor is for supported IPC;
    // otherwise, return NULL.
    SpIpcWorkerDelegate* GetWorker(int fd);

    // Get parameters from "write" functions.
    // Input Param : pt -- the call point from which we get the function
    // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
    // Output Param: buf_out -- the buffer to write, if NULL, then skip it
    // Output Param: c_out -- the character to write, if NULL, then skip it
    // Output Param: size_out -- the size of the buffer, if NULL, then skip it
    // Output Param: sa -- sockaddr for connect(), if NULL, then skip it
    void GetWriteParam(SpPoint* pt,
                       int* fd_out,
                       void** buf_out,
                       char* c_out,
                       size_t* size_out,
                       sockaddr** sa_out);

    // Get parameters from "read" functions.
    // Input Param : pt -- the call point from which we get the function
    // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
    // Output Param: buf_out -- the buffer to write, if NULL, then skip it
    // Output Param: size_out -- the size of the buffer, if NULL, then skip
    void GetReadParam(SpPoint* pt,
                      int* fd_out,
                      void** buf_out,
                      size_t* size_out);

    // Get parameters from "close" functions.
    void GetCloseParam(SpPoint* pt,
                       int* fd_out);

    //fcntl SETOWN to the socket descriptor if any
    void fcntlReturnParam(SpPoint* pt);
    // See if the function is a fork
    bool IsFork(const char* f);

    // See if the function is a popen
    bool IsPopen(const char* f);

    // See if current process is allowed to execute the payload code
    // This is used in the user-defined payload function.
    // Return 1 if it is allowed to execute payload code (for tracing);
    // otherwise, 0 is returned.
    char CanStartTracing(int fd);

    // Will be called before user-specified entry-payload function.
    static bool BeforeEntry(SpPoint*);

    // Will be called before user-specified exit-payload function.
    static bool BeforeExit(PointHandle*);

    // Get workers
    SpPipeWorker* pipe_worker() const { return pipe_worker_; }
    SpTcpWorker* tcp_worker() const { return tcp_worker_; }
    SpUdpWorker* udp_worker() const { return udp_worker_; }

    void HandleExec(SpPoint*);

 protected:

    // IPC workers
    SpPipeWorker* pipe_worker_;
    SpTcpWorker* tcp_worker_;
    SpUdpWorker* udp_worker_;

    // For the ease of iterating all workers
    typedef std::set<SpIpcWorkerDelegate*> WorkerSet;
    WorkerSet worker_set_;
  };


}

#endif /* _SPIPCMGR_H_ */
