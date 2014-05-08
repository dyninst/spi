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
#include <execinfo.h>
#include <libgen.h>
#include <linux/udp.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>


#include <fstream>
#include <vector>
#include <string>

#include "dynutil/h/util.h"
#include "common/utils.h"

namespace sp {

// -------------------------------------------------------------------
// Timer for profiling
// -------------------------------------------------------------------
typedef int64_t i64;
const int kNumTimers = 10;
typedef struct CPerfCounterRec {
  i64 _freq;
  i64 _clocks;
  i64 _start;
} CPerfCounter;
CPerfCounter a[kNumTimers];

//////////////////////////////////////////////////////////////////////

void
SetupTimer(const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return;
  }
  a[i]._clocks = 0;
  a[i]._start = 0;
  a[i]._freq = 1000;
  ResetTimer(i);
}

//////////////////////////////////////////////////////////////////////

void
StartTimer(const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return;
  }
  struct timeval s;
  gettimeofday(&s, 0);
  a[i]._start = (i64)s.tv_sec * 1000*1000 + (i64)s.tv_usec;
}

//////////////////////////////////////////////////////////////////////

void
StopTimer(const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return;
  }

  i64 n = 0;
  struct timeval s;
  gettimeofday(&s, 0);
  n = (i64)s.tv_sec * 1000 *1000 + (i64)s.tv_usec;
  n -= a[i]._start;
  a[i]._start = 0;
  a[i]._clocks += n;
}

//////////////////////////////////////////////////////////////////////

void
ResetTimer(const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return;
  }

  a[i]._clocks = 0;
}

//////////////////////////////////////////////////////////////////////

double
GetTimer(const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return 0.0;
  }

  return static_cast<double>(a[i]._clocks);
}

//////////////////////////////////////////////////////////////////////

void
PrintTime(const char *msg,
          const int i) {
  if (i < 0 || i > (kNumTimers - 1)) {
    sp_debug("OUT OF BOUND TIMER - timer id should be in [0, 9]");
    return;
  }

  if (msg != NULL)
    printf("%s: %f sec\n", msg, GetTimer(i));
  else
    printf("%f sec\n", GetTimer(i));
}

// ------------------------------------------------------------------- 
// Code generation utilities
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

// Determines the size of a displacement
bool
IsDisp32(const long d) {
  const int32_t max_int32 = 2147483646;
  const int32_t min_int32 = -2147483647;
  return ((d < max_int32) && (d >= min_int32));
}

//////////////////////////////////////////////////////////////////////

bool
IsDisp8(const long d) {
  const int8_t max_int8 = 127;
  const int8_t min_int8 = -128;
  return ((d < max_int8) && (d >= min_int8));
}

// ------------------------------------------------------------------- 
// Get pid utilities
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

// Gets inode from file descriptor
// Return -1 if failed to get an inode from this file descriptor
ino_t
GetInodeFromFileDesc(const int fd) {
  struct stat s;
  if (fstat(fd, &s) != -1) {
    return s.st_ino;
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////

// Is this process w/ `pid` using this `inode`?
bool
PidUsesInode(const int pid,
             const ino_t inode) {
//  sp_print("*** pid=%d uses inode=%lu?\n", pid, inode);
  DIR *dir;
  ino_t temp_node;
  struct dirent *de;
  char name[kLenStringBuffer];
  char buffer[kLenStringBuffer];
  snprintf(name, kLenStringBuffer, "/proc/%u/fd", pid);

  if ((dir = opendir(name)) == 0) {
    return false;
  }

  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      snprintf(name, kLenStringBuffer, "/proc/%u/fd/%s", pid, de->d_name);
      int size = -1;
      if ((size = readlink(name, buffer, kLenStringBuffer)) < 0) {
        sp_print("PidUsesInode: readlink error");
        return false;
      }
      buffer[size] = '\0';

  //    sp_print("***buffer=%s\n", buffer);
      if (sscanf(buffer, "pipe:[%lu]", &temp_node) == 1 &&
          temp_node == inode) {
        // Anonymous pipe
        closedir(dir);
        return true;
      } else if (sscanf(buffer, "socket:[%lu]", &temp_node) == 1 &&
                 temp_node == inode) {
        // Tcp
        closedir(dir);
        return true;
      } else {
        // Named pipe
        struct stat s;
        if (stat(buffer, &s) != -1) {
          if (s.st_ino == inode) {
            closedir(dir);
            return true;
          }
        }
      }   // Anoymous pipe? tcp? named pipe?
    }   // A process?
  }   // Iterate all processes
  closedir(dir);
  return false;
}

//////////////////////////////////////////////////////////////////////

// Gets all pids to `pid_set` that are using this file descriptor `fd`
void
GetPidsFromFileDesc(const int fd,
                    PidSet& pid_set) {
  int pid;
  DIR *dir;
  if ((dir = opendir("/proc")) == 0) {
    sp_perror("ERROR: cannot access /proc");
  }

  struct dirent *de;
  char *ep;
  while ((de = readdir(dir)) != 0) {

    if (isdigit(de->d_name[0])) {
      pid = strtol(de->d_name, &ep, 10);
      if (ep == 0 || *ep != 0 || pid < 0) {
        sp_perror("ERROR: strtol failed on %s\n", de->d_name);
      }
      if (pid != getpid() &&
          PidUsesInode(pid, GetInodeFromFileDesc(fd))) {
        sp_debug("GET PID FOR PIPE: pid - %d", pid);
        pid_set.insert(pid);
      }
    }
  }
  closedir(dir);
}
/////////////////////////////////////////////////////////////////////
//Get the sockets opened by a pid
////////////////////////////////////////////////////////////////////
void GetSocketDescFromPid(int pid, SocketSet& socket_set)
{
  DIR *dir;
  ino_t temp_node;
  struct dirent *de;
  char name[kLenStringBuffer];
  char buffer[kLenStringBuffer];
  snprintf(name, kLenStringBuffer, "/proc/%u/fd", pid);
  if ((dir = opendir(name)) == 0) {
    return ;
  }
  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      int dir=atoi(de->d_name);
      snprintf(name, kLenStringBuffer, "/proc/%u/fd/%s", pid, de->d_name);
      int size = -1;
      if ((size = readlink(name, buffer, kLenStringBuffer)) < 0) {
        sp_debug("GetSocketDescFromPid: readlink error");
      }
      else {
        buffer[size] = '\0';
        if (sscanf(buffer, "socket:[%lu]", &temp_node) == 1){
        // Tcp
        socket_set.insert(dir);
        sp_debug("%d uses Socket %d",pid,dir);
       }
     }
    }
  }   // Iterate all processes
  closedir(dir);
  return ;

}


//////////////////////////////////////////////////////////////////////

// Get pids that are associated with the remote address pair
// Basic ideas:
// 1. Look at /proc/net/tcp for inode
//    - TODO(wenbin): support udp
// 2. Iterate all pids, and see if it uses this inode

void
GetPidsFromAddrs(const char* const rem_ip,
                 const char* const rem_port,
                 PidSet& pid_set) {

  // Convert port string to int
  int rem_port_int = atoi(rem_port);
  sp_print("In GetPidsFromAddress: Remote ip_port is %d",rem_port_int);
  // fprintf(stderr, "*** look for port: %d / %x or %d / %x -> remote: %d / %x\n",
  //        rem_port_int, rem_port_int, rem_port_int+2, rem_port_int+2, 8001, 8001);
  
  // Read /proc/net/tcp for
  int inode = -1;
  int tried = 0;
  do {
    FILE* tcp_fp = fopen("/proc/net/tcp", "r");
    char line[2048];
    if (fgets(line, 2048, tcp_fp) == NULL) {
      sp_print("Failed to read headline of /proc/net/tcp");
    }
//    sp_print("Iterating through all the ports from /proc/net/tcp to find the inode corresponding to the remote port");
    while (fgets(line, 2048, tcp_fp) != NULL) {
      // fprintf(stderr, line);
      int iloc_ip, iloc_port, irem_ip, irem_port;
      if (sscanf(line, "%*u: %08X:%04X %08X:%04X %*02X %*08X:%*08X "
                 "%*02X:%*08X %*08X %*u %*d %u",
                 &iloc_ip, &iloc_port, &irem_ip, &irem_port, &inode) != 5) {
        sp_print("Failed to read a line in /proc/net/tcp");
      }
  //    sp_print("Port: %d", iloc_port);
      if (iloc_port == rem_port_int){ 
        sp_print("Found the port %d", iloc_port); 
        break;
      }
      else inode = -1;
    }
    fclose(tcp_fp);
    sp_print( "***inode=%d", inode);
    if (inode == -1) return;
    if (inode == 0 && tried < 5) {
      sleep(1000000);
      ++tried;
    }
    else break;
  } while (1);
  
  
  // Iterate all pids
  DIR *dir;
  int pid;
  if ((dir = opendir("/proc")) == 0) {
    sp_perror("ERROR: cannot access /proc");
  }
  struct dirent *de;
  char *ep;
  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      pid = strtol(de->d_name, &ep, 10);
      if (ep == 0 || *ep != 0 || pid < 0) {
        sp_perror("ERROR: strtol failed on %s\n", de->d_name);
      }
      if (pid != getpid() &&
          PidUsesInode(pid, inode)) {
        pid_set.insert(pid);
        // sp_print("Found pid=%d opening port %d", pid, rem_port_int);
      }
    } // Is pid?
  } // Iterate all pids
}

// ------------------------------------------------------------------- 
// Networking utilities
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

in_addr_t
GetIpFromHostname(const char* const hostname,
                  char* const ip,
                  const size_t ip_len) {
  struct hostent *he;
  struct in_addr **addr_list;
  int i;

  if ((he = gethostbyname(hostname)) == NULL) {
    perror("gethostbyname");
    return 0;
  }

  addr_list = (struct in_addr **) he->h_addr_list;
  for (i = 0; addr_list[i] != NULL; i++) {
    // Return the first one;
    strncpy(ip , inet_ntoa(*addr_list[i]), ip_len);
    sockaddr_in sa;
    if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != -1)
      return inet_lnaof(sa.sin_addr);
    return 0;
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////

// See if this file descriptor is for pipe.
bool
IsPipe(const int fd) {
  struct stat s;
  if (fstat(fd, &s) == -1) return false;
  if (S_ISFIFO(s.st_mode)) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

// See if this file descriptor is for tcp.
bool
IsTcp(const int fd) {
  struct stat st;
  int opt;
  socklen_t opt_len = sizeof(opt);

  if (fstat(fd, &st) < 0) {
    return false;
  }

  if (!S_ISSOCK(st.st_mode)) {
    return false;
  }

  // Here we try to probe if the socket is of the TCP kind. Couldn't
  // find the specific mechanism for that, so we'll simply ask for a
  // TCP-specific option.
  if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, &opt_len) < 0) {
    return false;    // Error is ok -- not a TCP socket
  }

  // Doublecheck that this is a stream socket.
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
    return false;
  }
  assert(opt == SOCK_STREAM);
  return true;
}

//////////////////////////////////////////////////////////////////////

// See if this file descriptor is for udp
bool
IsUdp(const int fd) {
  struct stat st;
  int opt;
  socklen_t opt_len = sizeof(opt);

  if (fstat(fd, &st) < 0) {
    return false;
  }
  if (!S_ISSOCK(st.st_mode)) {
    return false;
  }

  if (getsockopt(fd, IPPROTO_UDP, UDP_CORK, &opt, &opt_len) < 0) {
    return false;   // Error is ok -- not a UDP socket
  }

  // Doublecheck that this is a datagram socket.
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
    return false;
  }
  assert(opt == SOCK_DGRAM);
  return true;
}

//////////////////////////////////////////////////////////////////////

// See if the file descriptor is for any ipc mechanism
bool
IsIpc(const int fd) {
  return (IsPipe(fd) || IsTcp(fd) || IsUdp(fd));
}

//////////////////////////////////////////////////////////////////////

bool
GetLocalAddress(const int fd,
                sockaddr_storage* const out) {
  assert(out);
  socklen_t sock_len = sizeof(sockaddr_storage);
  if (getsockname(fd, reinterpret_cast<sockaddr*>(out), &sock_len) == -1) {
    sp_debug("FAILED GET LOCAL ADDR - getsockname @ pid = %d", getpid());
    return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

bool
GetRemoteAddress(const int fd,
                 sockaddr_storage* const out) {
  assert(out);
  socklen_t sock_len = sizeof(sockaddr_storage);
  if (getpeername(fd, reinterpret_cast<sockaddr*>(out), &sock_len) == -1) {
    sp_debug("FAILED GET REMOTE ADDR - getpeername @ pid = %d", getpid());
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////// 
pid_t
GetRemotePid(const char* host,
             const size_t host_len,
             const char* port,
             const size_t port_len) {
  return -1;
}

//////////////////////////////////////////////////////////////////////

bool
GetAddress(sockaddr_storage* const sa,
           char* const host,
           const size_t host_len,
           char* const service,
           const size_t service_len) {
  assert(host);
  assert(service);

  socklen_t sock_len = 0;
  if (sa->ss_family == AF_INET) {
    sock_len = sizeof(sockaddr_in);
  } else if (sa->ss_family == AF_INET6) {
    sock_len = sizeof(sockaddr_in6);
  } else if (sa->ss_family == AF_UNSPEC) {
    // sp_perror("AF_UNSPEC");
    sa->ss_family = AF_INET;
    sock_len = sizeof(sockaddr_in);
  }

  int err = 0;
  if ((err = getnameinfo((const sockaddr*)sa, sock_len, host, host_len,
                         service, service_len, NI_NUMERICSERV|NI_NUMERICHOST)) != 0) {
    fprintf(stderr, "%s\n", gai_strerror(err));
    perror("getnameinfo");
    return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

// Get local machine's IPv4 ip address (from outsider's view)
// From:
// http://stackoverflow.com/questions/212528/linux-c-get-the-ip-address-of-local-computer

void
GetIPv4Addr(char* buffer,
            size_t buflen) {
  assert(buflen >= 16);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  assert(sock != -1);

  const char* kGoogleDnsIp = "8.8.8.8";
  uint16_t kDnsPort = 53;
  struct sockaddr_in serv;
  memset(&serv, 0, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
  serv.sin_port = htons(kDnsPort);

  int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
  assert(err != -1);

  sockaddr_in name;
  socklen_t namelen = sizeof(name);
  err = getsockname(sock, (sockaddr*) &name, &namelen);
  assert(err != -1);

  const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
  assert(p);

  close(sock);
}

// ------------------------------------------------------------------- 
// /proc utilities
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

// Is current executable an illegal program?
// We use this, because we want to avoid instrumenting some programs, e.g.,
// unix utilities used in self-propelled core
bool
IsIllegalProgram() {
  StringSet illegal_exes;
  illegal_exes.insert("lsof");
  illegal_exes.insert("bash");
  illegal_exes.insert("injector.exe");
  illegal_exes.insert("sh");
  illegal_exes.insert("ssh");
  illegal_exes.insert("xauth");
  illegal_exes.insert("tcsh");
  illegal_exes.insert("scp");
  illegal_exes.insert("cp");
  illegal_exes.insert("netstat");

/*  std::string proc_path = "";
  proc_path += "/proc/";
  proc_path += Dyninst::itos(getpid());
  proc_path += "/cmdline";

  std::string content = GetFileText(proc_path.c_str());
  char* exe_name = sp_filename(content.c_str());
  sp_debug("exe: %s", exe_name); */
  std::string exe_name = GetExeName();

  for (StringSet::iterator si = illegal_exes.begin();
       si != illegal_exes.end(); si++) {
    // sp_debug("comparing: %s", (*si).c_str());
    if ((*si).compare(exe_name.c_str()) == 0) return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

// Gets current process's executable's full path name
std::string
GetExeName() {
  char exe[kLenStringBuffer];
  char link[kLenStringBuffer];
  int size = -1;
  snprintf(link,kLenStringBuffer,"/proc/%d/exe",getpid());
  if((size = readlink(link,exe,kLenStringBuffer)) < 0) {
        sp_perror("Cannot find the Executable name from /proc/pid/exe");
    } 
   exe[size] ='\0';

  sp_debug("Exename is %s",exe);
  std::string content(exe);

  return content;
}

//////////////////////////////////////////////////////////////////////

// Gets text content from a file. If file doesn't exist, return "".
std::string
GetFileText(const char* filename) {
  std::ifstream infile(filename);
  if (infile) {
    std::string fileData((std::istreambuf_iterator<char>(infile)),
                         std::istreambuf_iterator<char>());
    infile.close();
    return fileData;
  } else {
    return "";
  }
}
////////////////////////////////////////////////////////////////////
//Check if a process already contains the library
bool
ProcessHasLibrary(int pid, std::string lib)
{
   char maps_file[256];
   sprintf(maps_file, "/proc/%d/maps", pid);
   std::ifstream fp;
   fp.open(maps_file);
   if (!fp) {
     sp_perror("FAILED to open /proc/%d/maps file %s", pid, maps_file);
   }

  std::string line;
  while(std::getline(fp,line))
  {
      if(line.find(lib) != std::string::npos)
      {
        sp_debug("Agent shared library is already injected into the process");
        return true;
      }
 }
 return false;
}
//------------------------------------------------------------------
//Signal handler for SIGURG
//-----------------------------------------------------------------
void sig_urg_handler(int signo)
{

        int             n;
        char    buff[100];
 	int connfd;
	sp_debug("SIGURG received!!");
        SocketSet socket_set;
        GetSocketDescFromPid(getpid(),socket_set);
        if(socket_set.size() > 0) {
        for(SocketSet::iterator s=socket_set.begin(); s!=socket_set.end(); s++) {
   	   connfd=*s;
	   sp_debug("Socket: %d, trying to receive OOB", connfd);
           if ((n = recv(connfd, buff, sizeof(buff)-1, MSG_OOB)) > 0) {
		       buff[n] ='\0';
		       sp_print("read OOB data %s",buff);
	               break;
	}
       }
    }
   signal(SIGURG,sig_urg_handler);
}




// ------------------------------------------------------------------- 
// Serialization utilities
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

bool
serialize_co(pe::CodeObject* co,
             const char* dir) {
  assert(0 && "TODO");
  return false;
}

//////////////////////////////////////////////////////////////////////

pe::CodeObject*
deserialize_co(const char* dir,
               const char* file) {
  return NULL;
}


// -------------------------------------------------------------------
// Lock / Unlock
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

SpTid
GetThreadId() {
  return (SpTid)pthread_self();
}

//////////////////////////////////////////////////////////////////////

int
InitLock(SpLock* const lock) {
  assert(lock);
  sp_debug("INIT LOCK - mutex %d", lock->mutex);
  lock->mutex = 0;
  return 0;
}

//////////////////////////////////////////////////////////////////////

// The implementation is from:
/* http://stackoverflow.com/questions/1383363/is-my-spin-lock-implem  \
   entation-correct-and-optimal */

int Lock(SpLock* const lock) {
  assert(lock);
  while (__sync_lock_test_and_set(&lock->mutex, 1)) {
    while (lock->mutex);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////

int
Unlock(SpLock* const lock) {
  assert(lock);
  // sp_debug("UNLOCK - mutex %d", lock->mutex);
  __sync_synchronize();
  lock->mutex = 0;
  return 0;
}

//////////////////////////////////////////////////////////////////////

// Get Shared library
void*
GetSharedMemory(int id, size_t size) {
  int shmid;
  void* shm;
  if ((shmid = shmget(id, size, IPC_CREAT | 0666)) < 0) {
    sp_perror("Failed to create a shared memory w/ %lu bytes w/ id %d",
              (unsigned long)size, id);
  }
  if ((long)(shm = (void*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("Failed to get shared memory pointer");
  }
  return shm;
}

// ------------------------------------------------------------------- 
// For debugging
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

static void
sigseghandler(int signo) {
  sp_print("******************SIGSEGV*************************");

  void* array[10];
  size_t size;
  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, 2);
  exit(0);
}

//////////////////////////////////////////////////////////////////////

void
SetSegfaultSignal() {
  signal(SIGSEGV, sigseghandler);
}
//////////////////////////////////////////////////////////////////////

bool 
IsRecvLikeFunction(std::string func)
{
	if(func.compare("recv") == 0 )
		return true;
	return false;
}



}
