#include "SpIpcMgr.h"
#include "SpPoint.h"
#include "SpContext.h"
#include "SpInjector.h"

using ph::PatchFunction;

namespace sp {

extern SpContext* g_context;

#define TRACING_ID 1987
#define TRACING_SIZE 32768

/* =========================================================
                       IPC Manager
   ========================================================= */

SpIpcMgr::SpIpcMgr() {
  pipe_worker_ = new SpPipeWorker;
  worker_set_.insert(pipe_worker_);

  tcp_worker_ = new SpTcpWorker;
  worker_set_.insert(tcp_worker_);

  udp_worker_ = new SpUdpWorker;
  worker_set_.insert(udp_worker_);
}

SpIpcMgr::~SpIpcMgr() {

  delete pipe_worker_;
  delete tcp_worker_;
  delete udp_worker_;
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

int
SpIpcMgr::get_fd(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return -1;

  ArgumentHandle h;
  if (f->name().compare("write") == 0 ||
      f->name().compare("send") == 0) {
    int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
    return *fd;
  }

  if (f->name().compare("fputs") == 0) {
    char** str = (char**)sp::pop_argument(pt, &h, sizeof(char*));
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    return fileno(*fp);
  }

  if (f->name().compare("fputc") == 0) {
    char* c = (char*)sp::pop_argument(pt, &h, sizeof(char));
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    return fileno(*fp);
  }

  return -1;
}

bool
SpIpcMgr::is_sender(const char* f) {
  if (strcmp(f, "write") == 0 ||
      strcmp(f, "send") == 0 ||
      strcmp(f, "fputs") == 0 ||
      strcmp(f, "fputc") == 0
     )
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

char SpIpcMgr::start_tracing() {
  for (WorkerSet::iterator wi = worker_set_.begin(); wi != worker_set_.end(); wi++) {
    if ((*wi)->start_tracing()) return 1;
  }
  return 0;
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
   2. See if it is a pipe
   3. If so, see if we've injected
   4. If not, inject it
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
  int fd = ipc_mgr->get_fd(pt);
  if (fd == -1 || fd == 0 || fd == 1 || fd == 2) return true;

  SpIpcWorker* worker = NULL;

  /* PIPE */
  if (ipc_mgr->is_pipe(fd)) {
    worker = ipc_mgr->pipe_worker();
  }
  /* TCP */
  else if (ipc_mgr->is_tcp(fd)) {
    worker = ipc_mgr->tcp_worker();
  }
  /* UDP */
  else if (ipc_mgr->is_udp(fd)) {
    worker = ipc_mgr->udp_worker();
  } else {
    return true;
  }

  SpChannel* c = worker->get_channel(fd);
  if (c && c->remote_pid != 0) {
    /* A valid IPC channel. */
    sp_print("PIPE to: %d", c->remote_pid);
    worker->set_start_tracing(1, c->remote_pid);

    /* Inject this agent.so to remote process
       Luckily, the SpInjector implementation will automatically detect whether
       the agent.so library is already injected. If so, it will not inject the
       the library again.
    */
    worker->inject(c);
  }

  return true;
}

bool
SpIpcMgr::pre_after(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  /* Detect fork for pipe */
  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  if (ipc_mgr->is_fork(f->name().c_str())) {
    long pid = sp::retval(pt);
    /* Receiver */
    if (pid == 0) {
      sp_print("FORK - child pid = %d", getpid());
      ipc_mgr->pipe_worker()->set_start_tracing(0, getpid());
    } else {
    /* Sender */
      sp_print("FORK - parent pid = %d", getpid());
    }
  }
  /* Receipt-side */

  /* Detect receipt */
  return true;
}

/* =========================================================
                       IPC workers
   ========================================================= */

/* PIPE worker */
SpPipeWorker::SpPipeWorker() {
  tracing_internal(&start_tracing_);
  start_tracing_[getpid()] = 1;
}

SpPipeWorker::~SpPipeWorker() {
  for (ChannelMap::iterator i = channel_map_.begin(); i != channel_map_.end(); i++) {
    delete i->second;
  }
}

void
SpPipeWorker::tracing_internal(char** start_tracing) {
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

void  SpPipeWorker::set_start_tracing(char yes_or_no, pid_t pid) {
  start_tracing_[pid] = yes_or_no;
}

char SpPipeWorker::start_tracing() {
  return start_tracing_[getpid()];
}

/*
   1. If so, see if we've injected
   2. If not, inject it
*/
bool SpPipeWorker::inject(SpChannel* c) {
  if (c->injected) return true;
  sp_print("NO INJECTED -- start injection");
  SpInjector::ptr injector = SpInjector::create(c->remote_pid);
  string agent_name = g_context->parser()->get_agent_name();
  injector->inject(agent_name.c_str());
  c->injected = true;
  return true;
}

int
SpPipeWorker::pid_uses_inode(int pid, int inode) {

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

/* Get inode from file descriptor  */
long
SpPipeWorker::get_inode_from_fd(int fd) {
  struct stat s;
  if (fstat(fd, &s) != -1) {
    return s.st_ino;
  }
  return -1;
}

void
SpPipeWorker::get_pids_from_fd(int fd, PidSet& pid_set) {
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

SpChannel* SpPipeWorker::get_channel(int fd) {
  long inode = get_inode_from_fd(fd);
  if (channel_map_.find(inode) != channel_map_.end())
    return channel_map_[inode];

  SpChannel* c = new PipeChannel;
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
  c->inode = inode;
  channel_map_[inode] = c;

  return c;
}


/* TCP worker */
void  SpTcpWorker::set_start_tracing(char yes_or_no, pid_t pid) {
}

char SpTcpWorker::start_tracing() {
  return 0;
}

bool SpTcpWorker::inject(SpChannel*) {
  return 0;
}


SpChannel* SpTcpWorker::get_channel(int fd) {

}

/* UDP worker */
void  SpUdpWorker::set_start_tracing(char yes_or_no, pid_t pid) {
}

char SpUdpWorker::start_tracing() {
  return 0;
}

bool SpUdpWorker::inject(SpChannel*) {
  return 0;
}

SpChannel* SpUdpWorker::get_channel(int fd) {
  return NULL;
}

}
