#include "SpIpcMgr.h"
#include "SpPoint.h"
#include "SpContext.h"
#include "SpInjector.h"
#include <sys/wait.h>

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

/* See if this file descriptor is a tcp */
bool
SpIpcMgr::is_tcp(int fd) {
  struct stat st;
  int opt;
  socklen_t opt_len = sizeof(opt);

  if (fstat(fd, &st) < 0) {
    return false;
  }

  if (!S_ISSOCK(st.st_mode)) {
    return false;
  }

  /* Here we try to probe if the socket is of the TCP kind. Couldn't                                                                               
     find the specific mechanism for that, so we'll simply ask for a                                                                               
     TCP-specific option */
  if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, &opt_len) < 0) {
    return false; /* Error is ok -- not a TCP socket */
  }

  /* Doublecheck that this is a stream socket */
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
    return false;
  }
  assert(opt == SOCK_STREAM);
  return true;
}

/* See if this file descriptor is a udp */
bool
SpIpcMgr::is_udp(int fd) {
  struct stat st;
  int opt;
  socklen_t opt_len = sizeof(opt);

  if (fstat(fd, &st) < 0) {
    return false;
  }
  if (!S_ISSOCK(st.st_mode)) {
    return false;
  }
  /* Here we try to probe if the socket is of the UDP kind. Couldn't                                                                               
     find the specific mechanism for that, so we'll simply ask for a                                                                               
     UDP-specific option */
  if (getsockopt(fd, IPPROTO_UDP, UDP_CORK, &opt, &opt_len) < 0) {
    return false; /* Error is ok -- not a UDP socket */
  }

  /* Doublecheck that this is a datagram socket */
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
    return false;
  }
  assert(opt == SOCK_DGRAM);
  return true;
}

void
SpIpcMgr::get_write_param(SpPoint* pt, int* fd_out, void** buf_out,
                          char* c_out, size_t* size_out) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  ArgumentHandle h;
  if (f->name().compare("write") == 0 ||
      f->name().compare("send") == 0) {
    int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    void** buf = (void**)sp::pop_argument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = *buf;
    size_t* size = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = *size;
  }

  if (f->name().compare("fputs") == 0) {
    char** str = (char**)sp::pop_argument(pt, &h, sizeof(char*));
    if (buf_out) *buf_out = (void*)*str;
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  if (f->name().compare("fputc") == 0) {
    char* c = (char*)sp::pop_argument(pt, &h, sizeof(char));
    if (c_out) *c_out = *c;
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if (fd_out) *fd_out = fileno(*fp);
  }

  if (f->name().compare("fwrite_unlocked") == 0 ||
      f->name().compare("fwrite") == 0) {
    void** ptr = (void**)sp::pop_argument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = (void*)*ptr;
    size_t* size = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    size_t* n = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = (*size) * (*n);
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }
}

void SpIpcMgr::get_read_param(SpPoint* pt, int* fd_out, void** buf_out,
                              size_t* size_out) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  ArgumentHandle h;
  if (f->name().compare("read") == 0 ||
      f->name().compare("recv") == 0) {
    int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
    if (fd_out) *fd_out = *fd;
    void** buf = (void**)sp::pop_argument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = *buf;
    size_t* size = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = *size;
  }

  if (f->name().compare("fgets") == 0) {
    char** str = (char**)sp::pop_argument(pt, &h, sizeof(char*));
    if (buf_out) *buf_out = (void*)*str;
    int* size = (int*)sp::pop_argument(pt, &h, sizeof(int));
    if (size_out) *size_out = *size;
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  if (f->name().compare("fgetc") == 0) {
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }

  if (f->name().compare("fread_unlocked") == 0 ||
      f->name().compare("fread") == 0) {
    void** ptr = (void**)sp::pop_argument(pt, &h, sizeof(void*));
    if (buf_out) *buf_out = (void*)*ptr;
    size_t* size = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    size_t* n = (size_t*)sp::pop_argument(pt, &h, sizeof(size_t));
    if (size_out) *size_out = (*size) * (*n);
    FILE** fp = (FILE**)sp::pop_argument(pt, &h, sizeof(FILE*));
    if(fd_out) *fd_out = fileno(*fp);
  }
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

bool
SpIpcMgr::is_popen(const char* f) {
  if (strcmp(f, "popen") == 0) return true;
  return false;
}

char SpIpcMgr::start_tracing() {
  for (WorkerSet::iterator wi = worker_set_.begin(); wi != worker_set_.end(); wi++) {
    if ((*wi)->start_tracing()) return 1;
  }
  return 0;
}

SpIpcWorker* SpIpcMgr::get_worker(int fd) {
  /* PIPE */
  if (is_pipe(fd)) {
    return pipe_worker();
  }
  /* TCP */
  else if (is_tcp(fd)) {
    return tcp_worker();
  }
  /* UDP */
  else if (is_udp(fd)) {
    return udp_worker();
  } else {
    return NULL;
  }
}

/* Payload functions wrappers, which will be used only in IPC mode.
*/
bool
SpIpcMgr::pre_before(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

  /* ----------------------------------------
                  Sender-side
     ---------------------------------------*/
  /* Detect initiation of communication */
  int fd = -1;
  ipc_mgr->get_write_param(pt, &fd, NULL, NULL, NULL);
  if (fd != -1) {
    SpIpcWorker* worker = ipc_mgr->get_worker(fd);
    if (!worker) return false;

    SpChannel* c = worker->get_channel(fd, SP_WRITE);
    if (c) {
      worker->set_start_tracing(1, c->remote_pid);
      pt->set_channel(c);

      /* Inject this agent.so to remote process
         Luckily, the SpInjector implementation will automatically detect whether
	 the agent.so library is already injected. If so, it will not inject the
	 the library again.
      */
      if (c->remote_pid != -1) worker->inject(c);
    }
    return true;
  }

  /* ----------------------------------------
                  Receiver-side
     ---------------------------------------*/
  fd = -1;
  ipc_mgr->get_read_param(pt, &fd, NULL, NULL);
  if (fd != -1) {
    SpIpcWorker* worker = ipc_mgr->get_worker(fd);
    if (!worker) return false;
    SpChannel* c = worker->get_channel(fd, SP_READ);
    if (c) {
      pt->set_channel(c);
    }
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
      ipc_mgr->pipe_worker()->set_start_tracing(0, getpid());
    } else {
    /* Sender */
    }
  }
  /* Detect popen for pipe */
  else if (ipc_mgr->is_popen(f->name().c_str())) {
    FILE* fp = (FILE*)sp::retval(pt);
    int fd = fileno(fp);
    /* XXX: magic?? This is a very artificial way to wait for fork done */
    sleep(2);
    SpChannel* c = ipc_mgr->pipe_worker()->get_channel(fd, SP_WRITE);
    ipc_mgr->pipe_worker()->set_start_tracing(0, c->remote_pid);
  }

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
  for (ChannelMap::iterator i = channel_map_write_.begin();
       i != channel_map_write_.end(); i++) {
    delete i->second;
  }
  for (ChannelMap::iterator i = channel_map_read_.begin();
       i != channel_map_read_.end(); i++) {
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
  sp_debug("NO INJECTED -- start injection");
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
      int size = -1;
      if ((size = readlink(name, buffer, MAXLEN)) < 0) {
	perror("pid_uses_inode: readlink error");
	return (-1);
      }
      buffer[size] = '\0';

      /* Anonymous pipe */
      if (sscanf(buffer, "pipe:[%u]", &temp_node) == 1 &&
	  temp_node == inode) {
	closedir(dir);
	return 1;
      }
      /* Named pipe */
      else {
	struct stat s;
	if (stat(buffer, &s) != -1) {
	  if (s.st_ino == inode) {
	    closedir(dir);
	    return 1;
	  }
	}
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
      if (pid != getpid() && 
          pid_uses_inode(pid, get_inode_from_fd(fd))) {
	sp_debug("GET PID FOR PIPE: pid - %d", pid);
	pid_set.insert(pid);
      }
    }
  }
  closedir(dir);
}

SpChannel* SpPipeWorker::get_channel(int fd, ChannelRW rw) {
  long inode = get_inode_from_fd(fd);
  if (rw == SP_WRITE) {
    if (channel_map_write_.find(inode) != channel_map_write_.end())
      return channel_map_write_[inode];
  } else {
    if (channel_map_read_.find(inode) != channel_map_read_.end())
      return channel_map_read_[inode];
  }

  SpChannel* c = new PipeChannel;
  c->local_pid = getpid();
  PidSet pid_set;
  get_pids_from_fd(fd, pid_set);
#ifndef SP_RELEASE
  sp_debug("FD TO PID - get a %d pids from fd %d", pid_set.size(), fd);
#endif    
  for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
    if (*i != c->local_pid) {
      c->remote_pid = *i;
      break;
    }
  }
  c->type = SP_PIPE;

  c->inode = inode;

  if (rw == SP_WRITE) {
    c->rw = SP_WRITE;
    channel_map_write_[inode] = c;
#ifndef SP_RELEASE
    sp_debug("PIPE CHANNEL - get a WRITE pipe channel with inode %d for fd %d", inode, fd);
#endif    
  } else {
    c->rw = SP_READ;
    channel_map_read_[inode] = c;
#ifndef SP_RELEASE
    sp_debug("PIPE CHANNEL - get a READ pipe channel with inode %d for fd %d", inode, fd);
#endif    
  }
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


SpChannel* SpTcpWorker::get_channel(int fd, ChannelRW rw) {

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

SpChannel* SpUdpWorker::get_channel(int fd, ChannelRW rw) {
  return NULL;
}

}
