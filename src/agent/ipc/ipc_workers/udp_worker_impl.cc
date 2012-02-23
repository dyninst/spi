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
#include "agent/ipc/ipc_workers/udp_worker_impl.h"

namespace sp {

  // UDP worker
  void  SpUdpWorker::set_start_tracing(char yes_or_no, SpChannel* c) {
  }

  void SpUdpWorker::set_start_tracing(char yes_or_no) {
  }

  char SpUdpWorker::start_tracing(int fd) {
    return 0;
  }

  bool SpUdpWorker::inject(SpChannel* c, char* agent_path,
                           char* injector_path,
                           char* ijagent_path) {
    return 0;
  }

  SpChannel* SpUdpWorker::create_channel(int fd, ChannelRW rw, void*) {
    return NULL;
  }

} // Namespace sp
