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
#include "agent/ipc/ipc_workers/pipe_worker_impl.h"
#include "injector/injector.h"

namespace sp {

// Global variables
extern SpContext* g_context;
extern SpParser::ptr g_parser;

// For pipe's start_tracing() implementation (we use shared memory here).
#define TRACING_ID 1987
#define TRACING_SIZE 32768

//////////////////////////////////////////////////////////////////////

SpPipeWorker::SpPipeWorker() {
  // sp_debug("PIPE WORKER - created for pid=%d", getpid());
  TracingInternal(&start_tracing_);
  start_tracing_[getpid()] = 0;
}

//////////////////////////////////////////////////////////////////////

SpPipeWorker::~SpPipeWorker() {
  for (ChannelMap::iterator i = channel_map_write_.begin();
       i != channel_map_write_.end(); i++) {
    delete i->second;
  }
  for (ChannelMap::iterator i = channel_map_read_.begin();
       i != channel_map_read_.end(); i++) {
    delete i->second;
  }
}

//////////////////////////////////////////////////////////////////////

// Initialize shared memory
void
SpPipeWorker::TracingInternal(char** start_tracing) {
  int shmid;
  if ((shmid = shmget(TRACING_ID, TRACING_SIZE, IPC_CREAT | 0666)) < 0) {
    sp_perror("ERROR: cannot create shared memory with id %d", TRACING_ID);
  }

  char* shm = NULL;
  if ((long)(shm = (char*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("ERROR: cannot get shared memory");
  }
  *start_tracing = shm;
}

//////////////////////////////////////////////////////////////////////

void
SpPipeWorker::SetRemoteStartTracing(char yes_or_no,
                                    SpChannel* c) {
  start_tracing_[c->remote_pid] = yes_or_no;
}

//////////////////////////////////////////////////////////////////////

void
SpPipeWorker::SetLocalStartTracing(char yes_or_no) {
  start_tracing_[getpid()] = yes_or_no;
}

//////////////////////////////////////////////////////////////////////

char
SpPipeWorker::CanStartTracing(int fd) {
  sp_print("pipeworker:start_tracing: %d", start_tracing_[getpid()]);
  return start_tracing_[getpid()];
}

//////////////////////////////////////////////////////////////////////

// Invoke SpInjector::inject directly
bool
SpPipeWorker::Inject(SpChannel* c,
                     char* agent_path) {
  // XXX: potential problem - two hosts may communicate w/ multiple channels.
  //      e.g., pipe and tcp at the same time. Should have an approach to
  //      do bookkeeping correctly.
  if (c->injected) return true;

  sp_debug("NO INJECTED -- start injection");

  if (c->remote_pid > 0) {
     sp_debug("Remote pid is %d",c->remote_pid);
    if(ProcessHasLibrary(c->remote_pid, "libmyagent")){
     sp_debug("Process %d already has agent shared library", c->remote_pid);
     return true;
    }

    SpInjector::ptr injector = SpInjector::Create(c->remote_pid);
    string agent_name = "";
    if (getenv("SP_AGENT_DIR")) {
      agent_name = getenv("SP_AGENT_DIR");
      agent_name += "/";
    } else {
      agent_name = "./";
    }
    agent_name += g_parser->agent_name();
    sp_print("%s", agent_name.c_str());
    c->injected = injector->Inject(agent_name.c_str());
  }

  return c->injected;
}

//////////////////////////////////////////////////////////////////////

SpChannel*
SpPipeWorker::CreateChannel(int fd,
                            ChannelRW rw,
                            void*) {
  SpChannel* c = new PipeChannel;
  c->local_pid = getpid();
  PidSet pid_set;
  GetPidsFromFileDesc(fd, pid_set);
  sp_debug("FD TO PID - get a %lu pids from fd %d",
           (unsigned long)pid_set.size(), fd);
  if (pid_set.size() == 0) {
    sp_debug("FAILED TO GET PID - from fd = %d", fd);
    // return NULL;
  }

  for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
    if (*i != c->local_pid) {
      c->remote_pid = *i;
      break;
    }
  }
  c->type = SP_PIPE;
  return c;
}


} // Namespace sp
