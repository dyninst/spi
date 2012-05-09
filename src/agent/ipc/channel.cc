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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <set>
#include <string>
#include <vector>

#include "common/utils.h"
#include "agent/ipc/channel.h"

namespace sp {

// ------------------------------------------------------------------- 
// TcpChannel
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

static std::string
GetHost(sockaddr_storage ss) {
  sockaddr* sa = (sockaddr*)&ss;
  void* src = NULL;
  
  if (sa->sa_family == AF_INET) {
    // IPv4
    src = &(((struct sockaddr_in*)sa)->sin_addr);
  } else {
    // IPv6
    src = &(((struct sockaddr_in6*)sa)->sin6_addr);
  }

  char s[128];
  inet_ntop(ss.ss_family, src, s, 128);
  return s;
}

//////////////////////////////////////////////////////////////////////

std::string
TcpChannel::GetLocalHost() {
  return GetHost(local);
}

//////////////////////////////////////////////////////////////////////

std::string
TcpChannel::GetRemoteHost() {
  return GetHost(remote);
}

//////////////////////////////////////////////////////////////////////

static int
GetPort(sockaddr_storage sa) {
  sockaddr_in *sin;
  sockaddr_in6 *sin6;
  switch(sa.ss_family) {
    case AF_INET:
      sin = (struct sockaddr_in *)&sa;
      return ntohs(sin->sin_port);
    case AF_INET6:
      sin6 = (struct sockaddr_in6 *)&sa;
      return ntohs(sin6->sin6_port);
    default:
      return 0;
  }
}

//////////////////////////////////////////////////////////////////////

int
TcpChannel::GetLocalPort() {
  return GetPort(local);
}

//////////////////////////////////////////////////////////////////////

int
TcpChannel::GetRemotePort() {
  return GetPort(remote);
}

//////////////////////////////////////////////////////////////////////
pid_t
TcpChannel::GetRemotePid() {

  if (remote_pid == -1) {
    char port[32];
    snprintf(port, 32, "%d", GetRemotePort());
    sp::PidSet pid_set;
    sp::GetPidsFromAddrs(GetRemoteHost().c_str(), port, pid_set);
    remote_pid = *(pid_set.begin());
  }
  
  return remote_pid;
}

}
