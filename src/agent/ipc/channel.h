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

// This provides uni-directional channel
// Local process is the one that sends or writes to the channel
// Remote process is the one that receives or reads from the channel

#ifndef _SPCHANNEL_H_
#define _SPCHANNEL_H_

#include <sys/socket.h>

#include "common/common.h"

namespace sp {

typedef enum {
  SP_UNKNOWN = 0,
  SP_PIPE = 1,
  SP_TCP = 2,
  SP_UDP = 3
} ChannelType;

typedef enum {
  SP_READ,
  SP_WRITE,
  SP_NORW
} ChannelRW;


struct SpChannel {
  SpChannel() :
  type(SP_UNKNOWN), injected(false), local_pid(-1), remote_pid(-1),
    remote_injected(false), inode(-1), fd(-1), rw(SP_WRITE) {}

  ChannelType type;      // Channel type
  bool injected;         // Already injected?
  pid_t local_pid;       // Sender's pid
  pid_t remote_pid;      // Receiver's pid
  bool remote_injected;  // Agent is already injected to receiver?
  ino_t inode;           // System-wide inode number
  int fd;                // local fd that represents this channel
  ChannelRW rw;          // Read or Write?
};



// Assume only two parties are involved in a pipe, which is not true in real
// world. For instance, named pipe would have multiple processes involved.
// Need to consider it later.

struct PipeChannel : public SpChannel {
  PipeChannel() : SpChannel() {}
};


struct TcpChannel : public SpChannel {
  TcpChannel() :
  SpChannel() {}
  sockaddr_storage local;
  sockaddr_storage remote;

  pid_t GetRemotePid();
  std::string GetLocalHost();
  std::string GetRemoteHost();
  int GetLocalPort();
  int GetRemotePort();
};

struct UdpChannel : public SpChannel {
  /*
  UdpChannel() :
  SpChannel(), local_ip(0), local_port(0), remote_ip(0), remote_port(0) {}

  in_addr_t local_ip;
  uint16_t local_port;
  in_addr_t remote_ip;
  uint16_t remote_port;
  */
};

}
#endif /* _SPCHANNEL_H_ */
