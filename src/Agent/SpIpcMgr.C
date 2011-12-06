#include "SpIpcMgr.h"

namespace sp {

#define TRACING_ID 1987
#define TRACING_SIZE 32768

static void
tracing_internal(char** start_tracing) {
  int shmid;
  if ((shmid = shmget(TRACING_ID, TRACING_SIZE, IPC_CREAT | 0666)) < 0) {
    sp_perror("ERROR: cannot create shared memory with id %d", TRACING_ID);
  }

  char* shm = NULL;
  if ((long)(shm = (char*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("ERROR: cannot get shared memory");
  }
  *start_tracing = &shm[getpid()];
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

}
