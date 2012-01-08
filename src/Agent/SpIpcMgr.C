#include "SpPoint.h"
#include "SpUtils.h"
#include "SpIpcMgr.h"
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

    if (f->name().compare("accept") == 0) {
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
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
        worker->set_start_tracing(1, c);
        pt->set_channel(c);

        // Inject this agent.so to remote process
        // Luckily, the SpInjector implementation will automatically detect whether
        // the agent.so library is already injected. If so, it will not inject the
        // the library again.
        // if (c->remote_pid != -1) worker->inject(c);
				worker->inject(c);
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
        ipc_mgr->pipe_worker()->set_start_tracing(0);
      }
    }
    // Detect popen for pipe
    else if (ipc_mgr->is_popen(f->name().c_str())) {
      FILE* fp = (FILE*)sp::retval(pt);
      int fd = fileno(fp);
      // XXX: magic?? This is a very artificial way to wait for fork done
      sleep(2);
      SpChannel* c = ipc_mgr->pipe_worker()->get_channel(fd, SP_WRITE);
      ipc_mgr->pipe_worker()->set_start_tracing(0, c);
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
      sp_debug("WRITE CHANNEL - get a WRITE channel with inode %ld for fd %d", get_inode_from_fd(fd), fd);
#endif
    } else {
      c->rw = SP_READ;
      channel_map_read_[fd] = c;
#ifndef SP_RELEASE
      sp_debug("READ CHANNEL - get a READ channel with inode %ld for fd %d", get_inode_from_fd(fd), fd);
#endif
    }
    return c;
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

  void  SpPipeWorker::set_start_tracing(char yes_or_no, SpChannel* c) {
    start_tracing_[c->remote_pid] = yes_or_no;
  }

  void SpPipeWorker::set_start_tracing(char yes_or_no) {
    start_tracing_[getpid()] = yes_or_no;
	}

  char SpPipeWorker::start_tracing() {
    return start_tracing_[getpid()];
  }

  // Invoke SpInjector::inject directly
	bool SpPipeWorker::inject(SpChannel* c, char* agent_path,
														char* /* ignore injector for pipe */) {

		// XXX: potential problem - two hosts may communicate w/ multiple channels.
    //      e.g., pipe and tcp at the same time. Should have an approach to 
    //      do bookkeeping correctly.
    if (c->injected) return true; 
    sp_debug("NO INJECTED -- start injection");
    SpInjector::ptr injector = SpInjector::create(c->remote_pid);
    string agent_name = g_context->parser()->get_agent_name();
    injector->inject(agent_name.c_str());
    c->injected = true;
    return true;
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

	SpTcpWorker::SpTcpWorker() : start_tracing_(1) {
	}

  void SpTcpWorker::set_start_tracing(char yes_or_no, SpChannel*) {
		// TODO (wenbin): who will do this job? Injector??
		start_tracing_ = yes_or_no;
  }

  void SpTcpWorker::set_start_tracing(char yes_or_no) {
		start_tracing_ = yes_or_no;
	}

  char SpTcpWorker::start_tracing() {
    return start_tracing_;
  }

	bool SpTcpWorker::inject(SpChannel* c, char* agent_path,
													 char* injector_path) {

		// XXX: potential problem - two hosts may communicate w/ multiple channels.
    //      e.g., pipe and tcp at the same time. Should have an approach to 
    //      do bookkeeping correctly.
    if (c->injected) return true; 
    sp_debug("NO INJECTED -- start injection");

		TcpChannel *tcp_channel = static_cast<TcpChannel*>(c);
		assert(tcp_channel);

		// XXX: how to determine it is a local machiune?
    // 1. 127.0.0.1
    // 2. local ip == remote ip
    // ??
		bool local_machine = false;
		if (inet_netof(tcp_channel->remote_ip) == 127 ||
        inet_netof(tcp_channel->remote_ip) == 0) {
			local_machine = true;
		}
		sp_debug("REMOTE IP: %s (%d)", inet_ntoa(tcp_channel->remote_ip), tcp_channel->remote_ip.s_addr);
		sp_debug("REMOTE PORT: %d", tcp_channel->remote_port);

		// XXX: Should do it in a configure file
		if (agent_path == NULL) {
			assert(g_context);
			assert(g_context->parser());
			assert(g_context->parser()->get_agent_name().size() > 0);
		  agent_path = (char*)g_context->parser()->get_agent_name().c_str();
		}
		if (injector_path == NULL) {
			injector_path = (char*)"./Injector";
		}
		sp_debug("AGENT PATH - %s", agent_path);
		sp_debug("INJECTOR PATH - %s", injector_path);

    // 0. scp agent.so to /tmp/agent.so
		string cp_cmd;

		if (local_machine) cp_cmd = "cp -f "; else cp_cmd = "scp ";

		cp_cmd = cp_cmd + agent_path + " " + injector_path;

		if (local_machine) cp_cmd += " /tmp/";
		else cp_cmd = cp_cmd + " " + inet_ntoa(tcp_channel->remote_ip) + ":/tmp/";

		system(cp_cmd.c_str());

		// 1. SSH into remote machine to run Injector
		string exe_cmd;
		if (local_machine) {
			exe_cmd = "/tmp/Injector ";
		}
		else {
			exe_cmd = "ssh ";
			exe_cmd += inet_ntoa(tcp_channel->remote_ip);
			exe_cmd += " /tmp/";
			exe_cmd += sp_filename(injector_path);
			exe_cmd += " ";
		}
		char port_buf[255];
		exe_cmd += inet_ntoa(tcp_channel->local_ip);
		exe_cmd += " ";
		sprintf(port_buf, "%d", tcp_channel->local_port);
		exe_cmd += port_buf;
		exe_cmd += " ";
		exe_cmd += inet_ntoa(tcp_channel->remote_ip);
		exe_cmd += " ";
		sprintf(port_buf, "%d", tcp_channel->remote_port);
		exe_cmd += port_buf;
		exe_cmd += " /tmp/";
		exe_cmd += sp_filename(agent_path);

		sp_debug("INJECT CMD - %s", exe_cmd.c_str());
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



  SpChannel* SpTcpWorker::create_channel(int fd, ChannelRW rw, void* arg) {
    TcpChannel* c = new TcpChannel;
    c->local_pid = getpid();
    c->type = SP_TCP;
    c->inode = get_inode_from_fd(fd);
	
		char loc_ip[256];
	
    sockaddr_in rem_sa;
    if (arg != NULL) {
      // Get remote ip and port
      rem_sa = *(sockaddr_in*)arg;
      c->remote_ip = rem_sa.sin_addr;
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
			c->remote_ip = rem_sa.sin_addr;
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

		if (inet_netof(c->remote_ip) == 127) {
			c->local_ip = c->remote_ip;
		} // Intra-machine Communication

		else {
			char buf[256];
			if (gethostname(buf, 256) == -1) {
				perror("gethostname");
			}
			if (hostname_to_ip(buf, loc_ip, 256) == 0) {
				perror("hostname_to_ip");
			}
			// sp_print(loc_ip);
			if (inet_aton(loc_ip, &c->local_ip) == 0) {
				perror("inet_aton");
			}
		} // Inter-machine Communication
		/*
		sp_print("remote ip: %s (%d)", inet_ntoa(rem_sa.sin_addr), c->remote_ip.s_addr);
		sp_print("remote port: %d (%X)", htons(rem_sa.sin_port), htons(rem_sa.sin_port));
		sp_print("local ip: %s (%d)", inet_ntoa(c->local_ip), c->local_ip.s_addr);
		sp_print("local port: %d (%X)", htons(loc_sa.sin_port), htons(loc_sa.sin_port));
*/
    return c;
  }


  // UDP worker
  void  SpUdpWorker::set_start_tracing(char yes_or_no, SpChannel* c) {
  }

  void SpUdpWorker::set_start_tracing(char yes_or_no) {
	}

  char SpUdpWorker::start_tracing() {
    return 0;
  }

	bool SpUdpWorker::inject(SpChannel* c, char* agent_path,
 													 char* injector_path) {
    return 0;
  }

  SpChannel* SpUdpWorker::create_channel(int fd, ChannelRW rw, void*) {
    return NULL;
  }

} // Namespace sp
