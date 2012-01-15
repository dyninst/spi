#include "SpUtils.h"
#include <fstream>

namespace sp {

	// -----------------------------------------------------------------------------
	// Profiling tools
	// -----------------------------------------------------------------------------
  typedef long i64;
  typedef struct CPerfCounterRec {
    i64 _freq;
    i64 _clocks;
    i64 _start;
  } CPerfCounter;
  CPerfCounter a[10];

  void
  SetupTimer(int i) {
    a[i]._clocks = 0;
    a[i]._start = 0;
    a[i]._freq = 1000;
    ResetTimer(i);
  }

  void
  StartTimer(int i) {
    struct timeval s;
    gettimeofday(&s, 0);
    a[i]._start = (i64)s.tv_sec * 1000*1000 + (i64)s.tv_usec;
  }

  void
  StopTimer(int i) {
    i64 n = 0;
    struct timeval s;
    gettimeofday(&s, 0);
    n = (i64)s.tv_sec * 1000 *1000 + (i64)s.tv_usec;
    n -= a[i]._start;
    a[i]._start = 0;
    a[i]._clocks += n;
  }

  void
  ResetTimer(int i) {
    a[i]._clocks = 0;
  }

  double
  GetTimer(int i) {
    return (double)a[i]._clocks;
  }

  void
  PrintTime(char *msg, int i) {
    if (msg != NULL)
      printf("%s: %f sec\n", msg, GetTimer(i));
    else
      printf("%f sec\n", GetTimer(i));
  }


	// -----------------------------------------------------------------------------
	// Determine the size of a long integer
	// -----------------------------------------------------------------------------
  bool is_disp32(long d) {
    const long max_int32 = 2147483646;
    const long min_int32 = -2147483647;
    return ((d < max_int32) && (d >= min_int32));
  }

  bool is_disp8(long d) {
    const long max_int8 = 127;
    const long min_int8 = -128;
    return ((d < max_int8) && (d >= min_int8));
  }

	// -----------------------------------------------------------------------------
	// Get pid from various things
	// -----------------------------------------------------------------------------
  const int MAXLEN = 255;

  // Get inode from file descriptor
  // Return -1 if failed to get an inode from this file descriptor
	ino_t
	get_inode_from_fd(int fd) {
    struct stat s;
    if (fstat(fd, &s) != -1) {
      return s.st_ino;
    }
    return -1;
	}

  // Is this process using this inode?
  bool
  pid_uses_inode(int pid, ino_t inode) {

    DIR *dir;
    ino_t temp_node;
    struct dirent *de;
    char name[MAXLEN], buffer[MAXLEN];
    sprintf(name, "/proc/%u/fd", pid);

    if ((dir = opendir(name)) == 0) {
      return false;
    }

    while ((de = readdir(dir)) != 0) {
      if (isdigit(de->d_name[0])) {
        sprintf(name, "/proc/%u/fd/%s", pid, de->d_name);
        int size = -1;
        if ((size = readlink(name, buffer, MAXLEN)) < 0) {
          perror("pid_uses_inode: readlink error");
          return false;
        }
        buffer[size] = '\0';
				// sp_print(buffer);
        if (sscanf(buffer, "pipe:[%lu]", &temp_node) == 1 &&
            temp_node == inode) {
          closedir(dir);
          return true;
        } // Anonymous pipe

				else if (sscanf(buffer, "socket:[%lu]", &temp_node) == 1 &&
								 temp_node == inode) {
          closedir(dir);
          return true;
        } // tcp

        else {
          struct stat s;
          if (stat(buffer, &s) != -1) {
            if (s.st_ino == inode) {
              closedir(dir);
              return true;
            }
          }
        } // Named pipe
      }
    } // Iterate all processes
    closedir(dir);
    return false;
  }

  // Get all pids that are using this fd
  void
  get_pids_from_fd(int fd, PidSet& pid_set) {
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

	// Get pids that are associated with the local/remote address pair
	void addr_to_pids(char* loc_ip, char* loc_port,
										char* rem_ip, char* rem_port,
										PidSet& pid_set) {
		char cmd[1024];
		sprintf(cmd, "/usr/sbin/lsof -i UDP:%s -i TCP:%s", rem_port, rem_port);
		// system(cmd);

		FILE* fp = popen(cmd, "r");
		char line[1024];
		fgets(line, 1024, fp); // skip the header line
		// fprintf(stderr, "after pipe\n");

		while (fgets(line, 1024, fp) != NULL) {
			// fprintf(stderr, "%s\n", line);

			char* pch = strtok(line, " :()->");
			std::vector<char*> tokens;
			while (pch != NULL) {
				tokens.push_back(pch);
				pch = strtok(NULL, " :()->");
			}
			// fprintf(stderr, "%s == %s\n", tokens[8], rem_port);
			if (atoi(tokens[8]) == atoi(rem_port)) {
				pid_set.insert(atoi(tokens[1]));
			}
		}
		pclose(fp);

	}

	in_addr_t hostname_to_ip(char * hostname , char* ip, size_t ip_len) {
		struct hostent *he;
		struct in_addr **addr_list;
		int i;

		if ( (he = gethostbyname( hostname ) ) == NULL)	{
			perror("gethostbyname");
			return 0;
		}

		addr_list = (struct in_addr **) he->h_addr_list;
		for(i = 0; addr_list[i] != NULL; i++)	{
			//Return the first one;
			strncpy(ip , inet_ntoa(*addr_list[i]), ip_len);
			sockaddr_in sa;
			if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != -1)
				return inet_lnaof(sa.sin_addr);
			return 0;
		}
		return 0;
	}

// ----------------------------------------------------------------------------- 
// /proc utilities
// -----------------------------------------------------------------------------

	// Is current executable an illegal program?
	// We use this, because we want to avoid instrumenting some programs, e.g.,
	// unix utilities used in self-propelled core
	bool is_illegal_exe(StringSet& illegal_exes) {
		std::string proc_path = "";
		proc_path += "/proc/";
		proc_path += Dyninst::itos(getpid());
		proc_path += "/cmdline";

		std::string content = get_file_text(proc_path);
		char* exe_name = sp_filename(content.c_str());
		sp_debug("exe: %s", exe_name);

		for (StringSet::iterator si = illegal_exes.begin();
         si != illegal_exes.end(); si++) {
			sp_debug("comparing: %s", (*si).c_str());
			if ((*si).compare(exe_name) == 0) return true;
		}
		return false;
	}

	// Get text content from a file. If file doesn't exist, return "".
	std::string get_file_text(std::string filename) {
		std::ifstream infile(filename.c_str()) ;
		if ( infile ) {
			std::string fileData((std::istreambuf_iterator<char>(infile)) ,
														std::istreambuf_iterator<char>()) ;
      infile.close( ) ; ;
      return fileData;
		}
		else {
      return "";
		}
	}

// ----------------------------------------------------------------------------- 
// IPC stuffs
// -----------------------------------------------------------------------------	

  // See if this file descriptor is for pipe.
  bool
  is_pipe(int fd) {
    struct stat s;
    if (fstat(fd, &s) == -1) return false;
    if (S_ISFIFO(s.st_mode)) {
      return true;
    }
    return false;
  }

  // See if this file descriptor is for tcp.
  bool
  is_tcp(int fd) {
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
  is_udp(int fd) {
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

  // See if the file descriptor is for any ipc mechanism
  bool
  is_ipc(int fd) {
    return (is_pipe(fd) || is_tcp(fd) || is_udp(fd));
  }

// ----------------------------------------------------------------------------- 
// Socket programming things
// -----------------------------------------------------------------------------
	bool get_local_address(int fd, sockaddr_storage* out) {
		assert(out);
		socklen_t sock_len = sizeof(sockaddr_storage);
		if (getsockname(fd, (sockaddr*)out, &sock_len) == -1) {
			sp_perror("getsockname @ pid = %d", getpid());
		}
		return true;
	}

	bool get_remote_address(int fd, sockaddr_storage* out) {
		assert(out);
		socklen_t sock_len = sizeof(sockaddr_storage);
		if (getpeername(fd, (sockaddr*)out, &sock_len) == -1) {
			sp_perror("getpeername @ pid = %d", getpid());
		}
		return true;
	}

	bool get_address(sockaddr_storage* sa, char* host, size_t host_len,
									 char* service, size_t service_len) {
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
}
