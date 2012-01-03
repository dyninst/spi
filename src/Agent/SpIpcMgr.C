#include "SpIpcMgr.h"
#include "SpPoint.h"
#include "SpContext.h"
#include "SpInjector.h"
#include <sys/wait.h>

using ph::PatchFunction;

namespace sp {

  // Global variable for the only one SpContext instance.
  extern SpContext* g_context;

  // For pipe's start_tracing() implementation (we use shared memory here).
  #define TRACING_ID 1987
  #define TRACING_SIZE 32768

// -----------------------------------------------------------------------------
// IPC Manager
// -----------------------------------------------------------------------------
  // Constructor
  // Instantiate workers for different IPC mechanisms.
  SpIpcMgr::SpIpcMgr() {
    pipe_worker_ = new SpPipeWorker;
    worker_set_.insert(pipe_worker_);

    tcp_worker_ = new SpTcpWorker;
    worker_set_.insert(tcp_worker_);

    udp_worker_ = new SpUdpWorker;
    worker_set_.insert(udp_worker_);
  }

  // Destructor
  // Destroy all workers.
  SpIpcMgr::~SpIpcMgr() {
    delete pipe_worker_;
    delete tcp_worker_;
    delete udp_worker_;
  }


  // See if this file descriptor is for pipe.
  bool
  SpIpcMgr::is_pipe(int fd) {
    struct stat s;
    if (fstat(fd, &s) == -1) return false;
    if (S_ISFIFO(s.st_mode)) {
      return true;
    }
    return false;
  }

  // See if this file descriptor is for tcp.
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

    // Here we try to probe if the socket is of the TCP kind. Couldn't
    // find the specific mechanism for that, so we'll simply ask for a
    // TCP-specific option.
    if (getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, &opt_len) < 0) {
      return false; // Error is ok -- not a TCP socket
    }

    // Doublecheck that this is a stream socket.
    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
      return false;
    }
    assert(opt == SOCK_STREAM);
    return true;
  }

  // See if this file descriptor is for udp
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

    if (getsockopt(fd, IPPROTO_UDP, UDP_CORK, &opt, &opt_len) < 0) {
      return false; // Error is ok -- not a UDP socket
    }

    // Doublecheck that this is a datagram socket.
    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &opt, &opt_len) < 0) {
      return false;
    }
    assert(opt == SOCK_DGRAM);
    return true;
  }

  // Get parameters from "write" functions.
  // Input Param : pt -- the call point from which we get the function
  // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
  // Output Param: buf_out -- the buffer to write, if NULL, then skip it
  // Output Param: c_out -- the character to write, if NULL, then skip it
  // Output Param: size_out -- the size of the buffer, if NULL, then skip it
  // Output Param: sa_out -- sockaddr for connect(), if NULL, then skip it
  void
  SpIpcMgr::get_write_param(SpPoint* pt, int* fd_out, void** buf_out,
                            char* c_out, size_t* size_out, sockaddr** sa_out) {
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

    if (f->name().compare("connect") == 0) {
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
      sockaddr** sa = (sockaddr**)sp::pop_argument(pt, &h, sizeof(sockaddr*));
      if (sa_out) *sa_out = *sa;
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

  // Get parameters from "read" functions.
  // Input Param : pt -- the call point from which we get the function
  // Output Param: fd_out -- file descriptor, if it is NULL, then skip it
  // Output Param: buf_out -- the buffer to write, if NULL, then skip it
  // Output Param: size_out -- the size of the buffer, if NULL, then skip it
  void
  SpIpcMgr::get_read_param(SpPoint* pt, int* fd_out, void** buf_out,
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

  // See if the file descriptor is for any ipc mechanism
  bool
  SpIpcMgr::is_ipc(int fd) {
    return (is_pipe(fd) || is_tcp(fd) || is_udp(fd));
  }

  // See if the function is a fork
  bool
  SpIpcMgr::is_fork(const char* f) {
    if (strcmp(f, "fork") == 0) return true;
    return false;
  }

  // See if the function is a popen
  bool
  SpIpcMgr::is_popen(const char* f) {
    if (strcmp(f, "popen") == 0) return true;
    return false;
  }

  // See if current process is allowed to execute the payload code
  // This is used in the user-defined payload function.
  // Return 1 if it is allowed to execute payload code (for tracing); otherwise,
  // 0 is returned.
  char SpIpcMgr::start_tracing() {
    for (WorkerSet::iterator wi = worker_set_.begin();
         wi != worker_set_.end(); wi++) {
      if ((*wi)->start_tracing()) return 1;
    }
    return 0;
  }

  // Get a worker according to the file descriptor.
  // Return the worker if the file descriptor is for supported IPC; otherwise,
  // return NULL.
  SpIpcWorker* SpIpcMgr::get_worker(int fd) {
    // PIPE
    if (is_pipe(fd)) {
      return pipe_worker();
    }
    // TCP
    else if (is_tcp(fd)) {
      return tcp_worker();
    }
    // UDP
    else if (is_udp(fd)) {
      return udp_worker();
    }
    // No IPC
    else {
      return NULL;
    }
  }

  // Payload functions wrappers, which will be called before user-specified
  // entry-payload function.
  bool
  SpIpcMgr::before_entry(SpPoint* pt) {
    PatchFunction* f = sp::callee(pt);
    if (!f) return false;

    sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();

    // Sender-side
    // Detect initiation of communication
    int fd = -1;
    sockaddr* sa = NULL;
    ipc_mgr->get_write_param(pt, &fd, NULL, NULL, NULL, &sa);
    if (fd != -1) {
      SpIpcWorker* worker = ipc_mgr->get_worker(fd);
      if (!worker) return false;

      SpChannel* c = worker->get_channel(fd, SP_WRITE, sa);
      if (c) {
        worker->set_start_tracing(1, c->remote_pid);
        pt->set_channel(c);

        // Inject this agent.so to remote process
        // Luckily, the SpInjector implementation will automatically detect whether
        // the agent.so library is already injected. If so, it will not inject the
        // the library again.
        if (c->remote_pid != -1) worker->inject(c);
      }
      return true;
    }

    // Receiver-side
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

  // Payload functions wrappers, which will be called before user-specified
  // exit-payload function.
  bool
  SpIpcMgr::before_exit(SpPoint* pt) {
    PatchFunction* f = sp::callee(pt);
    if (!f) return false;

    // Detect fork for pipe
    sp::SpIpcMgr* ipc_mgr = sp::g_context->ipc_mgr();
    if (ipc_mgr->is_fork(f->name().c_str())) {
      long pid = sp::retval(pt);
      // Receiver
      if (pid == 0) {
        ipc_mgr->pipe_worker()->set_start_tracing(0, getpid());
      }
    }
    // Detect popen for pipe
    else if (ipc_mgr->is_popen(f->name().c_str())) {
      FILE* fp = (FILE*)sp::retval(pt);
      int fd = fileno(fp);
      // XXX: magic?? This is a very artificial way to wait for fork done
      sleep(2);
      SpChannel* c = ipc_mgr->pipe_worker()->get_channel(fd, SP_WRITE);
      ipc_mgr->pipe_worker()->set_start_tracing(0, c->remote_pid);
    }
    // Detect connect for tcp
    else {
    }
    return true;
  }

// -----------------------------------------------------------------------------
// IPC workers
// -----------------------------------------------------------------------------

  // Get channel from fd
  // If channel doesn't exist, construct one
  // Return NULL if failed to create one channel
  SpChannel* SpIpcWorker::get_channel(int fd, ChannelRW rw, void* arg) {
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

    // Construct one channel.
    SpChannel* c = create_channel(fd, rw, arg);

    // Update cache.
    if (rw == SP_WRITE) {
      c->rw = SP_WRITE;
      channel_map_write_[fd] = c;
#ifndef SP_RELEASE
      sp_debug("PIPE CHANNEL - get a WRITE pipe channel with inode %ld for fd %d", get_inode_from_fd(fd), fd);
#endif
    } else {
      c->rw = SP_READ;
      channel_map_read_[fd] = c;
#ifndef SP_RELEASE
      sp_debug("PIPE CHANNEL - get a READ pipe channel with inode %ld for fd %d", get_inode_from_fd(fd), fd);
#endif
    }
    return c;
  }

  // Get inode from file descriptor
  // Return -1 if failed to get an inode from this file descriptor
  long
  SpIpcWorker::get_inode_from_fd(int fd) {
    struct stat s;
    if (fstat(fd, &s) != -1) {
      return s.st_ino;
    }
    return -1;
  }

  // PIPE worker
  // Constructor
  SpPipeWorker::SpPipeWorker() {
    tracing_internal(&start_tracing_);
    start_tracing_[getpid()] = 1;
  }

  // Destructor
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

  // Initialize shared memory
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

  // Invoke SpInjector::inject directly
  bool SpPipeWorker::inject(SpChannel* c) {
    if (c->injected) return true;
    sp_debug("NO INJECTED -- start injection");
    SpInjector::ptr injector = SpInjector::create(c->remote_pid);
    string agent_name = g_context->parser()->get_agent_name();
    injector->inject(agent_name.c_str());
    c->injected = true;
    return true;
  }

  // Is this process using this inode?
  int
  SpPipeWorker::pid_uses_inode(int pid, int inode) {

    const int MAXLEN = 255;
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

        if (sscanf(buffer, "pipe:[%u]", &temp_node) == 1 &&
            temp_node == inode) {
          closedir(dir);
          return 1;
        } // Anonymous pipe

        else {
          struct stat s;
          if (stat(buffer, &s) != -1) {
            if ((long)s.st_ino == (long)inode) {
              closedir(dir);
              return 1;
            }
          }
        } // Named pipe
      }
    } // Iterate all processes
    closedir(dir);

    return 0;
  }

  // Get all pids that are using this fd
  void
  SpPipeWorker::get_pids_from_fd(int fd, PidSet& pid_set) {
    int pid;
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


  SpChannel* SpPipeWorker::create_channel(int fd, ChannelRW rw, void*) {
    SpChannel* c = new PipeChannel;
    c->local_pid = getpid();
    PidSet pid_set;
    get_pids_from_fd(fd, pid_set);
#ifndef SP_RELEASE
    sp_debug("FD TO PID - get a %lu pids from fd %d", pid_set.size(), fd);
#endif
    for (PidSet::iterator i = pid_set.begin(); i != pid_set.end(); i++) {
      if (*i != c->local_pid) {
        c->remote_pid = *i;
        break;
      }
    }
    c->type = SP_PIPE;
    return c;
  }

  void  SpTcpWorker::set_start_tracing(char yes_or_no, pid_t pid) {
  }

  char SpTcpWorker::start_tracing() {
    return 0;
  }

  bool SpTcpWorker::inject(SpChannel*) {
    // 0. scp agent.so to /tmp/agent.so
		// 1. SSH into remote machine to run SpServer 
    // 2. SpServer uses local ip/port and remote ip/port to look up remote pid
    // 3. SpServer injects /tmp/agent.so into remote process
    
    return 0;
  }

  SpChannel* SpTcpWorker::create_channel(int fd, ChannelRW rw, void* arg) {
    TcpChannel* c = new TcpChannel;
    c->local_pid = getpid();
    c->type = SP_TCP;
    c->inode = get_inode_from_fd(fd);

    sockaddr_in rem_sa;
    if (arg != NULL) {
      // Get remote ip and port
      rem_sa = *(sockaddr_in*)arg;
      c->remote_ip = inet_lnaof(rem_sa.sin_addr);
      c->remote_port = htons(rem_sa.sin_port);

			// Should bind from the client side, so that we can use getsockname
      // to get local ip/port
			sockaddr_in tmp_sa;
			memset(&tmp_sa, 0, sizeof(sockaddr_in));
			tmp_sa.sin_family = AF_INET;
			tmp_sa.sin_addr.s_addr = INADDR_ANY;
			if (bind(fd, (sockaddr*)&tmp_sa, sizeof(sockaddr)) == -1) {
				perror("bind");
			}
    } // Connect

    else {
			memset(&rem_sa, 0, sizeof(sockaddr_in));
			socklen_t rem_len = sizeof(sockaddr_in);
			if (getsockname(fd, (sockaddr*)&rem_sa, &rem_len) == -1) {
				perror("getsockname");
			}
			c->remote_ip = inet_lnaof(rem_sa.sin_addr);
			c->remote_port = htons(rem_sa.sin_port);
    } // Send / write

		// Get local ip/port
		sockaddr_in loc_sa;
		memset(&loc_sa, 0, sizeof(sockaddr_in));
		socklen_t loc_len = sizeof(sockaddr_in);
		if (getsockname(fd, (sockaddr*)&loc_sa, &loc_len) == -1) {
			perror("getsockname");
		}
		c->local_port = htons(loc_sa.sin_port);
		// XXX: how to get local ip correctly??
		// c->local_ip = inet_lnaof(loc_sa.sin_addr);
		if (c->remote_ip == 1) c->local_ip = 1;

		/*
		sp_print("remote ip: %s (%d)", inet_ntoa(rem_sa.sin_addr), c->remote_ip);
		sp_print("remote port: %d", htons(rem_sa.sin_port));
		sp_print("local ip: %s (%d)", inet_ntoa(loc_sa.sin_addr), c->local_ip);
		sp_print("local port: %d", htons(loc_sa.sin_port));
		*/
    return c;
  }


  // UDP worker
  void  SpUdpWorker::set_start_tracing(char yes_or_no, pid_t pid) {
  }

  char SpUdpWorker::start_tracing() {
    return 0;
  }

  bool SpUdpWorker::inject(SpChannel*) {
    return 0;
  }

  SpChannel* SpUdpWorker::create_channel(int fd, ChannelRW rw, void*) {
    return NULL;
  }

} // Namespace sp
