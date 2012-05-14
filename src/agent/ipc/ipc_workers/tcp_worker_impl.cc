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
#include "agent/ipc/ipc_workers/tcp_worker_impl.h"

namespace sp {

// Global variables
extern SpContext* g_context;
extern SpParser::ptr g_parser;

//////////////////////////////////////////////////////////////////////

SpTcpWorker::SpTcpWorker() : start_tracing_(0) {
  // sp_debug("TCP WORKER - created for pid=%d", getpid());
}

//////////////////////////////////////////////////////////////////////

void
SpTcpWorker::set_start_tracing(char yes_or_no,
                               SpChannel* c) {
  sp_debug("SET TRACING - yes_or_no (%d), fd (%d)", yes_or_no, c->fd);
  assert(c);
  // Sanity check
  if (c && IsTcp(c->fd)) {
    uint8_t mark_byte = (getpid() & 0xFF) | 1;
    sp_debug("OOB MARK - sending %x via fd=%d", mark_byte, c->fd);
    if (send(c->fd, &mark_byte, sizeof(mark_byte), MSG_OOB) < 0) {
      perror("send");
      sp_perror("OUT-OF-BAND - failed to send oob byte");
    }
  }
}

//////////////////////////////////////////////////////////////////////

void
SpTcpWorker::set_start_tracing(char yes_or_no) {
  start_tracing_ = yes_or_no;
}

//////////////////////////////////////////////////////////////////////

// If tcpworker has more than one read-channel, and it is not allowed to
// start tracing, then we need to wait for OOB msg
char
SpTcpWorker::start_tracing(int fd) {

  if (IsTcp(fd) && !start_tracing_) {
    fd_set rset, xset;
    FD_ZERO(&rset);
    FD_ZERO(&xset);
    for (; ;) {
      FD_SET(fd, &rset);
      FD_SET(fd, &xset);
      select(fd+1, &rset, NULL, &xset, NULL);
      if (FD_ISSET(fd, &xset)) {
        uint8_t mark = 0;
        recv(fd, &mark, sizeof(mark), MSG_OOB);
        if (mark != 0) start_tracing_ = 1;
        break;
      }
    }
  }
  return start_tracing_;
}

//////////////////////////////////////////////////////////////////////

// This interface is subject to change, which implies local and remote
// machines are binary compatible. However, it is not true. We may define a
// bunch of environment variables.
bool
SpTcpWorker::inject(SpChannel* c,
                    char* agent_path,
                    char* injector_path,
                    char* ijagent_path) {
  sp_debug("tcp worker, injected? %d", c->injected);
  // XXX: potential problem - two hosts may communicate w/ multiple channels.
  //      e.g., pipe and tcp at the same time. Should have an approach to
  //      do bookkeeping correctly.
  if (c->injected) return true;
  sp_debug("NO INJECTED -- start injection");

  TcpChannel *tcp_channel = static_cast<TcpChannel*>(c);
  assert(tcp_channel);

  char local_ip[256];
  char local_port[64];
  char remote_ip[256];
  char remote_port[64];

  if (!GetAddress(&tcp_channel->local, local_ip, 256, local_port, 64)) {
    sp_perror("failed to get local address in tcp_worker::inject()");
  }
  if (!GetAddress(&tcp_channel->remote, remote_ip, 256, remote_port, 64)) {
    sp_perror("failed to get remote address in tcp_worker::inject()");
  }

  // XXX: how to determine it is a local machiune?
  // 1. 127.0.0.1
  // 2. local ip == remote ip
  // ??
  bool local_machine = false;
  if (strstr(remote_ip, "127.0.0.1")) {
    sp_debug("LOCAL MACHINE TCP");
    local_machine = true;
  }
  sp_debug("REMOTE IP: %s, REMOTE PORT: %s", remote_ip, remote_port);

  // XXX: Should do it in a configure file
  string default_agent_path;
  string default_injector_path;
  string default_ijagent_path;

  if (agent_path == NULL) {
    assert(g_context);
    assert(g_parser);
    assert(g_parser->agent_name().size() > 0);

    if (getenv("SP_AGENT_DIR")) {
      default_agent_path += getenv("SP_AGENT_DIR");
      default_agent_path += "/";
    } else {
      default_agent_path = "./";
    }

    default_agent_path += sp_filename((char*)g_parser->agent_name().c_str());

    agent_path = (char*)default_agent_path.c_str();
  }
  if (injector_path == NULL) {
    default_injector_path += getenv("SP_DIR");
    default_injector_path += "/";
    default_injector_path += getenv("PLATFORM");
    default_injector_path += "/injector";
    injector_path = (char*)default_injector_path.c_str();
  }
  if (ijagent_path == NULL) {
    default_ijagent_path += getenv("SP_DIR");
    default_ijagent_path += "/";
    default_ijagent_path += getenv("PLATFORM");
    default_ijagent_path += "/libijagent.so";
    ijagent_path = (char*)default_ijagent_path.c_str();
  }

  sp_debug("AGENT PATH - %s", agent_path);
  sp_debug("INJECTOR PATH - %s", injector_path);
  sp_debug("IJAGENT PATH - %s", ijagent_path);

  // SSH into remote machine to run Injector
  string exe_cmd;
  if (local_machine) {
    exe_cmd = injector_path;
    exe_cmd += " ";
  }
  else {
    exe_cmd = "ssh ";
    exe_cmd += remote_ip;
    exe_cmd += " \"";
    exe_cmd += injector_path;
    exe_cmd += " ";
  }
  exe_cmd += local_ip;
  exe_cmd += " ";
  exe_cmd += local_port;
  exe_cmd += " ";
  exe_cmd += remote_ip;
  exe_cmd += " ";
  exe_cmd += remote_port;
  exe_cmd += " ";
  exe_cmd += agent_path;
  if (!local_machine)
    exe_cmd += "\"";

  // sp_debug("INJECT CMD - %s", exe_cmd.c_str());

  FILE* fp = popen(exe_cmd.c_str(), "r");
  char line[1024];
  fgets(line, 1024, fp);
  fgets(line, 1024, fp);
  if (strstr(line, "INJECTED") != NULL) {
    c->injected = true;
  }
  pclose(fp);

  return true;
}

//////////////////////////////////////////////////////////////////////

SpChannel*
SpTcpWorker::create_channel(int fd,
                            ChannelRW rw,
                            void* arg) {
  TcpChannel* c = new TcpChannel;
  c->local_pid = getpid();
  c->type = SP_TCP;
  c->inode = GetInodeFromFileDesc(fd);

  // connect, we can get remote ip/port from arg
  if (arg != NULL) {

    // Get local ip / port
    char host[256];
    char service[64];
    c->remote = *((sockaddr_storage*)arg);
    if (GetAddress(&c->remote, host, 256, service, 64)) {
      sp_debug("connect remote host: %s, service: %s\n", host, service);
    } else {
      sp_perror("failed to get connect remote address");
    }

    // Get local ip / port (skip it for now)
    if (GetLocalAddress(fd, &c->local)) {
      if (GetAddress(&c->local, host, 256, service, 64)) {
        sp_debug("connect local host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get local address for write/send");
      }
    } else {
    }
  }

  // send/write
  else if (rw == SP_WRITE || rw == SP_READ) {
    // Get remote ip / port
    if (GetRemoteAddress(fd, &c->remote)) {
      char host[256];
      char service[64];
      if (GetAddress(&c->remote, host, 256, service, 64)) {
        sp_debug("write/send remote host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get remote address for write/send");
      }
    } // remote address

    // Get local ip / port
    if (GetLocalAddress(fd, &c->local)) {
      char host[256];
      char service[64];
      if (GetAddress(&c->local, host, 256, service, 64)) {
        sp_debug("write/send local host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get local address for write/send");
      }
    } // remote address
  }
  return c;
}

} // Namespace sp
