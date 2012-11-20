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
#include "agent/ipc/ipc_workers/ipc_worker_delegate.h"
#include "agent/patchapi/point.h"
#include "common/utils.h"
#include "injector/injector.h"

namespace sp {

// Global variables
extern SpContext* g_context;
extern SpParser::ptr g_parser;

//////////////////////////////////////////////////////////////////////

// Get channel from fd
// If channel doesn't exist, construct one
// Return NULL if failed to create one channel
SpChannel*
SpIpcWorkerDelegate::GetChannel(int fd,
                                ChannelRW rw,
                                void* arg) {
  /*
  // Look up cache.
  if (rw == SP_WRITE) {
    if (channel_map_write_.find(fd) != channel_map_write_.end()) {
      SpChannel* c = channel_map_write_[fd];
      // Fill in the missed information
      if (c && c->type == SP_TCP) {
        // TcpChannel* tcp_channel = (TcpChannel*)c;
        // TODO
      }
      return c;
    }
  } else {
    if (channel_map_read_.find(fd) != channel_map_read_.end())
      return channel_map_read_[fd];
  }
  */
  // Construct one channel.
  SpChannel* c = CreateChannel(fd, rw, arg);
  if (!c) return NULL;

  // Update cache.
  if (rw == SP_WRITE) {
    c->rw = SP_WRITE;
    channel_map_write_[fd] = c;
    sp_debug("WRITE CHANNEL @ pid = %d - get a WRITE channel with"
             " inode %ld for fd %d", getpid(), GetInodeFromFileDesc(fd), fd);
  } else {
    c->rw = SP_READ;
    channel_map_read_[fd] = c;
    sp_debug("READ CHANNEL @ pid = %d - get a READ channel with"
             " inode %ld for fd %d", getpid(), GetInodeFromFileDesc(fd), fd);
  }
  c->fd = fd;
  return c;
}


} // Namespace sp
