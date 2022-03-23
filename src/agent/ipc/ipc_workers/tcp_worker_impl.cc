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

#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "agent/context.h"
#include "agent/ipc/ipc_workers/tcp_worker_impl.h"
#include <fcntl.h>

namespace sp {

// Global variables
extern SpContext* g_context;
extern SpParser::ptr g_parser;

//////////////////////////////////////////////////////////////////////

SpTcpWorker::SpTcpWorker() : start_tracing_(0) {
  // sp_debug_ipc("TCP WORKER - created for pid=%d", getpid());
}

//////////////////////////////////////////////////////////////////////

void
SpTcpWorker::SetRemoteStartTracing(char yes_or_no,
                             SpChannel* c) {

  /*if(c->send_oob)
	return;
  c->send_oob=true;
  sp_debug_ipc("SET TRACING - yes_or_no (%d), fd (%d)", yes_or_no, c->fd);
  assert(c);
  // Sanity check
  if (c && IsTcp(c->fd)) {
    //int mark_byte = (getpid() & 0xFF) | 1;
    sp_debug_ipc("OOB MARK \"S\" - sending via fd=%d", c->fd);
    if (send(c->fd, "S", 1, MSG_OOB) < 0) {
      perror("send");
      sp_perror("OUT-OF-BAND - failed to send oob byte");
    } else {
      sp_print("OUT-OF-BAND - mark \"S\" sent");
    }
  }*/
}

//////////////////////////////////////////////////////////////////////

void
SpTcpWorker::SetLocalStartTracing(char yes_or_no) {
  start_tracing_ = yes_or_no;
}

//////////////////////////////////////////////////////////////////////
char
SpTcpWorker::CanStartTracing(int fd) {

  /*if (start_tracing_) {
    // fprintf(stderr, "OK to trace\n");
    return start_tracing_;
  }
  
  int at_mark = 0;
  if (ioctl(fd, SIOCATMARK, &at_mark) < 0) {
    sp_debug_ipc("ioctl(SIOCATMARK) error");
    return 0;
  }
  if (at_mark) {
    uint8_t mark = 0;
    if (recv(fd, &mark, sizeof(mark), MSG_OOB) < 0) {
      sp_debug_ipc("Recv OOB error");
      return 0;
    }
    sp_debug_ipc("GOT mark at_mark=%d, mark=%d", at_mark, mark);
    // fprintf(stderr, "GOT mark at_mark=%d, mark=%d\n", at_mark, mark);
    start_tracing_ = 1;
    return 1;
  }*/
  return start_tracing_;
}

//////////////////////////////////////////////////////////////////////
// This interface is subject to change, which implies local and remote
// machines are binary compatible. However, it is not true. We may define a
// bunch of environment variables.
bool
SpTcpWorker::Inject(SpChannel* c,
                    char* agent_path) {

  if (c->injected) return true;
  sp_debug_ipc("NO INJECTED(tcp) -- start injection");

  TcpChannel *tcp_channel = static_cast<TcpChannel*>(c);
  assert(tcp_channel);

  // Get ip and port
  char remote_ip[256];
  char remote_port[64];
  if (!GetAddress(&tcp_channel->remote, remote_ip, 256, remote_port, 64)) {
    sp_perror("failed to get remote address in tcp_worker::inject()");
  }
  sp_debug_ipc("REMOTE IP: %s, REMOTE PORT: %s", remote_ip, remote_port);

  char cmd[1024];
  string cmd_exe;

  if (strstr(remote_ip, "127.0.0.1")) {
    // For local machine, invoke injector directly
    sp_debug_ipc("LOCAL MACHINE TCP");

    // Injector path
    if (getenv("SP_DIR") && getenv("PLATFORM")) {
      cmd_exe += "$SP_DIR/$PLATFORM";
    } else {
      cmd[0] = '\0';
    }
    cmd_exe += "/injector.exe";

    // IP and Port
    snprintf(cmd, 1024, " port %s", remote_port);
    cmd_exe += cmd;

    // Agent lib path
    if (getenv("SP_AGENT_DIR")) {
      cmd_exe += " $SP_AGENT_DIR";
    } else {
      cmd_exe += " ./";
    }

    // Agent lib name
    assert(g_parser);
    snprintf(cmd, 1024, "/%s",
         sp_filename((char*)g_parser->agent_name().c_str()));
    cmd_exe += cmd;
  } else {
    // For remote machine, ssh injector
    snprintf(cmd, 1024, "ssh %s -t \'bash -l -c \"", remote_ip);
    cmd_exe += cmd;

    // Injector path
    cmd_exe += "\\$SP_DIR/\\$PLATFORM";
    cmd_exe += "/injector.exe";

    // IP and Port
    snprintf(cmd, 1024, " port %s", remote_port);
    cmd_exe += cmd;

    // Agent lib path
    cmd_exe += " \\$SP_AGENT_DIR";

    // Agent lib name
    assert(g_parser);
    snprintf(cmd, 1024, "/%s",
         sp_filename((char*)g_parser->agent_name().c_str()));
    cmd_exe += cmd;

    cmd_exe += "\"\' 2>&1 | tee -a ./tmp/injector_log";
  }

  sp_debug_ipc("INJECT CMD -- %s", cmd_exe.c_str());

  // unset LD_PRELOAD before popen injector
  // then set LD_PRELOAD back after popen is finished
  std::string libs;
  if (char* ld_preload = std::getenv("LD_PRELOAD")) {
    libs = ld_preload;
    if (unsetenv("LD_PRELOAD") < 0) {
      sp_perror("unsetenv for LD_PRELOAD failed");
    }
  }

  // Execute the command
  FILE* fp = popen(cmd_exe.c_str(), "r");
  if (fp == NULL) {
    sp_debug_ipc("Popen error");
    return false;
  }
  char line[1024];
  while (fgets(line,sizeof(line),fp)) {
    if (strstr(line, "SUCCESS") != NULL) {
      sp_debug_ipc("Injected");
      c->injected = true;
      break;
    }
  }
  pclose(fp);
  sp_debug_ipc("Popen finished Returning %d",c->injected?1 :0);

  if (setenv("LD_PRELOAD", libs.c_str(), true) < 0) {
    sp_perror("setenv for LD_PRELOAD failed");
  }
  return c->injected;
}

//////////////////////////////////////////////////////////////////////

SpChannel*
SpTcpWorker::CreateChannel(int fd,
                           ChannelRW rw,
                           void* arg) {
  TcpChannel* c = new TcpChannel;
  c->local_pid = getpid();
  c->type = SP_TCP;
  c->inode = GetInodeFromFileDesc(fd);
  fcntl(fd, F_SETOWN, getpid());
  sp_debug_ipc("CREATE CHANNEL -- for fd=%d", fd);
  // connect, we can get remote ip/port from arg
  if (arg != NULL) {
    sp_debug_ipc("GET ADDR from sockaddr_storage");
    // Get local ip / port
    char host[256];
    char service[64];
    c->remote = *((sockaddr_storage*)arg);
    if (GetAddress(&c->remote, host, 256, service, 64)) {
      sp_debug_ipc("connect remote host: %s, service: %s\n", host, service);
    } else {
      sp_perror("failed to get connect remote address");
    }

    // Get local ip / port (skip it for now)
    if (GetLocalAddress(fd, &c->local)) {
      if (GetAddress(&c->local, host, 256, service, 64)) {
        sp_debug_ipc("connect local host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get local address for write/send");
      }
    } else {
    }
  }

  // send/write
  else if (rw == SP_WRITE || rw == SP_READ) {
    sp_debug_ipc("GET ADDR from getpeername");
    
    // Get remote ip / port
    if (GetRemoteAddress(fd, &c->remote)) {
      char host[256];
      char service[64];
      if (GetAddress(&c->remote, host, 256, service, 64)) {
        sp_debug_ipc("write/send remote host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get remote address for write/send");
      }
    } // remote address

    // Get local ip / port
    if (GetLocalAddress(fd, &c->local)) {
      char host[256];
      char service[64];
      if (GetAddress(&c->local, host, 256, service, 64)) {
        sp_debug_ipc("write/send local host: %s, service: %s\n", host, service);
      } else {
        sp_perror("failed to get local address for write/send");
      }
    } // remote address
  }
  return c;
}

} // Namespace sp
