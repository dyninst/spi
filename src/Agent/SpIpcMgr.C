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

			/*
			if (sa_out && is_tcp(*fd)) {
				sp_debug("GET REM_SA - for fd %d at a %s ()", *fd, f->name().c_str());
				sockaddr_in rem_sa;
    		memset(&rem_sa, 0, sizeof(sockaddr_in));
				socklen_t rem_len = sizeof(sockaddr_in);
				if (getpeername(*fd, (sockaddr*)&rem_sa, &rem_len) == -1) {
					perror("getsockname");
				}
				sp_debug("REMOTE IP - %s", inet_ntoa(rem_sa.sin_addr));
				sp_debug("REMOTE PORT - %d", htons(rem_sa.sin_port));
			}
			*/
    }

    if (f->name().compare("connect") == 0) {
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      if (fd_out) *fd_out = *fd;
      sockaddr** sa = (sockaddr**)sp::pop_argument(pt, &h, sizeof(sockaddr*));
			// sockaddr* sa_tmp = (sockaddr*)malloc(sizeof(sockaddr));
			// memcpy(sa_tmp, *sa, sizeof(sockaddr));
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
  char SpIpcMgr::start_tracing(int fd) {
    for (WorkerSet::iterator wi = worker_set_.begin();
         wi != worker_set_.end(); wi++) {
			// sp_debug("START TRACING - %d for pid=%d", (*wi)->start_tracing(), getpid());
			// fprintf(stderr, "START TRACING - %d for pid=%d\n", (*wi)->start_tracing(), getpid());
      if ((*wi)->start_tracing(fd)) return 1;
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
			// Enable tracing for current process
			worker->set_start_tracing(1);

      SpChannel* c = worker->get_channel(fd, SP_WRITE, sa);
      if (c) {
        // Inject this agent.so to remote process
        // Luckily, the SpInjector implementation will automatically detect whether
        // the agent.so library is already injected. If so, it will not inject the
        // the library again.
        // if (c->remote_pid != -1) worker->inject(c);
				worker->inject(c);

				// Enable tracing for remote process
				if (callee(pt)->name().compare("connect") != 0)
					worker->set_start_tracing(1, c);
        pt->set_channel(c);
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
		assert(c);

    // Update cache.
    if (rw == SP_WRITE) {
      c->rw = SP_WRITE;
      channel_map_write_[fd] = c;
#ifndef SP_RELEASE
      sp_debug("WRITE CHANNEL @ pid = %d - get a WRITE channel with inode %ld for fd %d", getpid(), get_inode_from_fd(fd), fd);
#endif
    } else {
      c->rw = SP_READ;
      channel_map_read_[fd] = c;
#ifndef SP_RELEASE
      sp_debug("READ CHANNEL @ pid = %d - get a READ channel with inode %ld for fd %d", getpid(), get_inode_from_fd(fd), fd);
#endif
    }
		c->fd = fd;
    return c;
  }


  // PIPE worker
  // Constructor
  SpPipeWorker::SpPipeWorker() {
		// sp_debug("PIPE WORKER - created for pid=%d", getpid());
    tracing_internal(&start_tracing_);
    start_tracing_[getpid()] = 0;
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
		// fprintf(stderr, "PipeWorker remote [pid=%d] -start tracing %d\n", c->remote_pid, yes_or_no);
  }

  void SpPipeWorker::set_start_tracing(char yes_or_no) {
    start_tracing_[getpid()] = yes_or_no;
		// fprintf(stderr, "PipeWorker local [pid=%d] -start tracing %d\n", getpid(), yes_or_no);
	}

  char SpPipeWorker::start_tracing(int fd) {
		// fprintf(stderr, "PipeWorker query local [pid=%d] -start tracing %d\n", getpid(), start_tracing_[getpid()]);
    return start_tracing_[getpid()];
  }

  // Invoke SpInjector::inject directly
	bool SpPipeWorker::inject(SpChannel* c, char* agent_path,
														char* /* ignore injector for pipe */,
														char* /* ignore ijagent for pipe */) {

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
    sp_debug("FD TO PID - get a %lu pids from fd %d", (unsigned long)pid_set.size(), fd);
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

	SpTcpWorker::SpTcpWorker() : start_tracing_(0) {
		// sp_debug("TCP WORKER - created for pid=%d", getpid());
	}

  void SpTcpWorker::set_start_tracing(char yes_or_no, SpChannel* c) {
		sp_debug("SET TRACING - yes_or_no (%d), fd (%d)", yes_or_no, c->fd);
		assert(c);
		// Sanity check
		if (c && is_tcp(c->fd)) {
			uint8_t mark_byte = (getpid() & 0xFF) | 1;
			sp_debug("OOB MARK - sending %x via fd=%d", mark_byte, c->fd);
			if (send(c->fd, &mark_byte, sizeof(mark_byte), MSG_OOB) < 0) {
				perror("send");
				sp_perror("OUT-OF-BAND - failed to send oob byte");
			}
			// fprintf(stderr,
			//				"TcpWorker remote [fd=%d] - start tracing %d, sending mark %x\n",
			//				c->fd, start_tracing_, mark_byte);
		}
  }

  void SpTcpWorker::set_start_tracing(char yes_or_no) {
		start_tracing_ = yes_or_no;
		// fprintf(stderr, "TcpWorker local [pid=%d] - start tracing %d\n", getpid(), start_tracing_);
	}

	/*
	// Out-of-band (OOB) handler
  int g_oob_fd = -1;
	char* g_start_tracing = NULL;
	void oob_handler(int sig) {
		uint8_t mark;
		if (sockatmark(g_oob_fd)) {
			if (recv(g_oob_fd, &mark, sizeof(mark), MSG_OOB) < 0 || mark == 0) {
				perror("recv");
				sp_perror("failed to recv MSG_OOB\n");
			}
			if (g_start_tracing) *g_start_tracing = 1;
			// fprintf(stderr, "got mark=%x\n", mark);
		}
	}
	*/

	// If tcpworker has more than one read-channel, and it is not allowed to
  // start tracing, then we need to wait for OOB msg
  char SpTcpWorker::start_tracing(int fd) {

		if (is_tcp(fd) && !start_tracing_) {
			/*
			g_oob_fd = fd;
			signal(SIGURG, oob_handler);
			fcntl(fd, F_SETOWN, getpid());
			g_start_tracing = &start_tracing_;

			sp_debug("WAIT FOR OOB - more than 1 channel, "
							 "and not allowed to trace for pid=%d", getpid());
			oob_handler(0);
			*/
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
					//fprintf(stderr,
					//				"TcpWorker query local [pid=%d, fd=%d] - start tracing %d w/ mark=%x\n",
          //        getpid(), fd, start_tracing_, mark);
					break;
				}
			}
			// fprintf(stderr, "WAIT FOR OOB - more than 1 channel, "
			//				"and not allowed to trace for pid=%d\n", getpid());
		}
    return start_tracing_;
  }

	// This interface is subject to change, which implies local and remote
	// machines are binary compatible. However, it is not true. We may define a
	// bunch of environment variables.
	bool SpTcpWorker::inject(SpChannel* c, char* agent_path,
													 char* injector_path,
													 char* ijagent_path) {
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

		if (!get_address(&tcp_channel->local, local_ip, 256, local_port, 64)) {
			sp_perror("failed to get local address in tcp_worker::inject()");
		}
		if (!get_address(&tcp_channel->remote, remote_ip, 256, remote_port, 64)) {
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
			assert(g_context->parser());
			assert(g_context->parser()->get_agent_name().size() > 0);
			default_agent_path += getenv("SP_DIR");
			default_agent_path += "/";
			default_agent_path += getenv("PLATFORM");
			default_agent_path += "/";
			default_agent_path += sp_filename((char*)g_context->parser()->get_agent_name().c_str());
		  agent_path = (char*)default_agent_path.c_str();
		}
		if (injector_path == NULL) {
			default_injector_path += getenv("SP_DIR");
			default_injector_path += "/";
			default_injector_path += getenv("PLATFORM");
			default_injector_path += "/Injector";
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
			exe_cmd += " ";
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

		sp_debug("INJECT CMD - %s", exe_cmd.c_str());
		// system(exe_cmd.c_str());
		// return true;
		// system("ssh feta /sbin/ifconfig");
		// system("ssh feta cd /tmp;/tmp/Injector 128.105.166.35 3490 128.105.167.125 56360 /tmp/ipc_test_agent.so");
		// c->injected = true;

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

		// connect, we can get remote ip/port from arg
		if (arg != NULL) {

			// Get local ip / port
			char host[256];
			char service[64];
			c->remote = *((sockaddr_storage*)arg);
			if (get_address(&c->remote, host, 256, service, 64)) {
				sp_debug("connect remote host: %s, service: %s\n", host, service);
			} else {
				sp_perror("failed to get connect remote address");
			}

			// Get local ip / port (skip it for now)
			if (get_local_address(fd, &c->local)) {
				if (get_address(&c->local, host, 256, service, 64)) {
					sp_debug("connect local host: %s, service: %s\n", host, service);
				} else {
					sp_perror("failed to get local address for write/send");
				}
			} else {
			}
		}

		// send/write
		else if (rw == SP_WRITE) {
			// Get remote ip / port
			if (get_remote_address(fd, &c->remote)) {
				char host[256];
				char service[64];
				if (get_address(&c->remote, host, 256, service, 64)) {
					sp_debug("write/send remote host: %s, service: %s\n", host, service);
				} else {
					sp_perror("failed to get remote address for write/send");
				}
			} // remote address

			// Get local ip / port
			if (get_local_address(fd, &c->local)) {
				char host[256];
				char service[64];
				if (get_address(&c->local, host, 256, service, 64)) {
					sp_debug("write/send local host: %s, service: %s\n", host, service);
				} else {
					sp_perror("failed to get local address for write/send");
				}
			} // remote address
		}
    return c;
  }


  // UDP worker
  void  SpUdpWorker::set_start_tracing(char yes_or_no, SpChannel* c) {
  }

  void SpUdpWorker::set_start_tracing(char yes_or_no) {
	}

  char SpUdpWorker::start_tracing(int fd) {
    return 0;
  }

	bool SpUdpWorker::inject(SpChannel* c, char* agent_path,
 													 char* injector_path,
													 char* ijagent_path) {
    return 0;
  }

  SpChannel* SpUdpWorker::create_channel(int fd, ChannelRW rw, void*) {
    return NULL;
  }

} // Namespace sp
