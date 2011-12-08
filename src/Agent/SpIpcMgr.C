#include "SpIpcMgr.h"
#include "SpPoint.h"
#include "SpContext.h"

using ph::PatchFunction;

namespace sp {

extern SpContext* g_context;

#define TRACING_ID 1987
#define TRACING_SIZE 32768

void
SpIpcMgr::tracing_internal(char** start_tracing) {
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

SpIpcMgr::SpIpcMgr() {
  tracing_internal(&start_tracing_);
  start_tracing_[getpid()] = 1;
}

SpIpcMgr::~SpIpcMgr() {
  for (ChannelMap::iterator i = channel_map_.begin(); i != channel_map_.end(); i++) {
    destroy_channel(i->second);
  }
}

/* Get inode from file descriptor  */
long
SpIpcMgr::get_inode_from_fd(int fd) {
  struct stat s;
  if (fstat(fd, &s) != -1) {
    return s.st_ino;
  }
  return -1;
}

/* See if this file descriptor is a pipe */
bool
SpIpcMgr::is_pipe(int fd) {
  struct stat s;
  if (fstat(fd, &s) == -1) return false;
  if (S_ISFIFO(s.st_mode)) {
    return true;
  }
  return false;
}

/* Get IPC channel from a file descriptor. 
   Return: NULL if not a valid IPC channel; otherwise, the channel.
*/
SpChannel*
SpIpcMgr::get_channel(int fd) {
  long inode = get_inode_from_fd(fd);
  if (channel_map_.find(inode) != channel_map_.end())
    return channel_map_[inode];

  SpChannel* c = NULL;

  /* PIPE */
  if (is_pipe(fd)) {
    c = new PipeChannel;
    c->local_pid = getpid();
    PidSet pid_set;
    get_pids_from_fd(fd, pid_set);
    for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
      if (*i != c->local_pid) {
	c->remote_pid = *i;
	break;
      }
    }
    c->type = SP_PIPE;
#ifndef SP_RELEASE
    sp_debug("PIPE CHANNEL - get a pipe channel with inode %d for fd %d", inode, fd);
#endif    
  }

  /* TCP */
  else if (is_tcp(fd)) {
    c = new TcpChannel;
    c->type = SP_TCP;
  }

  /* UDP */
  else if (is_udp(fd)) {
    c = new UdpChannel;
    c->type = SP_UDP;
  }

  if (c) {
    c->inode = inode;
    channel_map_[inode] = c;
    return c;
  }
  return NULL;
}

void
SpIpcMgr::destroy_channel(SpChannel* c) {
  delete c;
}

bool
SpIpcMgr::is_sender(const char* f) {
  if (strcmp(f, "write") == 0 ||
      strcmp(f, "send") == 0)
     return true;

  return false;
}

bool
SpIpcMgr::is_receiver(const char* f) {
  if (strcmp(f, "read") == 0 ||
      strcmp(f, "recv") == 0)
     return true;

  return false;
}

bool
SpIpcMgr::is_ipc(int fd) {
  return (is_pipe(fd) || is_tcp(fd) || is_udp(fd));
}

bool
SpIpcMgr::is_fork(const char* f) {
  if (strcmp(f, "fork") == 0) return true;
  return false;
}

static int
pid_uses_inode(int pid, int inode) {

#define MAXLEN 255
  DIR *dir;
  int temp_node;
  struct dirent *de;
  char name[MAXLEN], buffer[MAXLEN];
  sprintf(name, "/proc/%u/fd", pid);

  if ((dir = opendir(name)) == 0) {
    return 0;
  }

  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
    sprintf(name, "/proc/%u/fd/%s", pid, de->d_name);
    if (readlink(name, buffer, MAXLEN) < 0) {
	perror("pid_uses_inode: readlink error");
	return (-1);
    }
    if (sscanf(buffer, "pipe:[%u]", &temp_node) == 1 &&
	temp_node == inode) {
	closedir(dir);
	return 1;
    }
    }
  }
  closedir(dir);

    return 0;
}

void
SpIpcMgr::get_pids_from_fd(int fd, PidSet& pid_set) {
  int pid, rv, num_found = 0;
  DIR *dir;
  char *ep;
  struct dirent *de;

  if ((dir = opendir("/proc")) == 0) {
    sp_perror("ERROR: cannot access /proc");
  }
  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      pid = strtol(de->d_name, &ep, 10);
      if (ep == 0 || *ep != 0 || pid < 0) {
        sp_perror("ERROR: strtol failed on %s\n", de->d_name);
      }
      if (pid_uses_inode(pid, get_inode_from_fd(fd))) {
	pid_set.insert(pid);
      }
    }
  }
  closedir(dir);
}


char
SpIpcMgr::start_tracing() {
  return start_tracing_[getpid()];
}

void
SpIpcMgr::set_start_tracing(char b, int pid) {
  start_tracing_[pid] = b;
}

/* Payload functions wrappers, which will be used only in multi-process mode.

   === For pipe ===

   ** Uninstrumented case
      [Sender]                     [Receiver]
   1. fork receiver
   2. write to receiver
   3.                              read from sender
   4. write to receiver
   5.                              read from sender

   ** Before fork case
   If we use preload, or we happen to inject agent before 1:
      [Sender]                      [Receiver]
   1. Detect fork at pre_after
   2. Disable tracing for child
   3. Detect write/send for pipe
   4. Enable tracing for child
   5.                               Start tracing

   ** After fork case
   If we inject agent after 1:
      [Sender]                      [Receiver]
   1. Detect write/send for pipe
*/
bool
SpIpcMgr::pre_before(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  /* Sender-side */
  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  /* Detect initiation of communication */
  if (!ipc_mgr->is_sender(f->name().c_str())) {
#ifndef SP_RELEASE
    sp_debug("NON IPC FUNC - %s() is not a write or send", f->name().c_str());
#endif
    return true;
  }
#ifndef SP_RELEASE
  sp_debug("POTENTIAL IPC - %s() is a write or send", f->name().c_str());
#endif

  /* Get destination name */
  ArgumentHandle h;
  int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
  SpChannel* c = ipc_mgr->get_channel(*fd);

  if (!c) {
    /* Not a valid IPC channel. */
  } else {
    /* A valid IPC channel. */
  }

  /* Inject this agent.so to remote process */
  // ipc_mgr->inject(c);

  if (ipc_mgr->start_tracing()) {
    if (ipc_mgr->is_pipe(*fd)) {
      sp_print("PIPE to: %d", c->remote_pid);
      ipc_mgr->set_start_tracing(1, c->remote_pid);
    }
  }
  return true;
}

bool
SpIpcMgr::pre_after(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  /* Sender-side: detect fork for pipe */

  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  if (ipc_mgr->is_fork(f->name().c_str())) {
    long pid = sp::retval(pt);
    /* Receiver */
    if (pid == 0) {
      ipc_mgr->set_start_tracing(0, getpid());
    }
  }
  /* Receipt-side */

  /* Detect receipt */

/*
  // Handle fork, thus pipe
  if (f->name().compare("fork") == 0) {
    long pid = sp::retval(pt);
    if (pid == 0) {
      sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
      ipc_mgr->set_work(0, getpid());
      // Maintain a 32KB shared memory per machine
      // The creation of this shared memory is by SpServer
      // 1 byte for each process's can_work
      // We can support ~32000 processes, which is the maximum for linux
    } else if (pid > 0) {
      g_pid = pid;
    }
  }
*/
  return true;
}

}
