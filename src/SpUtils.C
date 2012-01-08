#include "SpUtils.h"

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
	void addr_to_pids(in_addr_t loc_ip, uint16_t loc_port,
										in_addr_t rem_ip, uint16_t rem_port,
										PidSet& pid_set) {
		char cmd[1024];
		sprintf(cmd, "/usr/sbin/lsof -i UDP:%d -i TCP:%d", rem_port, rem_port);
		FILE* fp = popen(cmd, "r");
		char line[1024];
		fgets(line, 1024, fp); // skip the header line

		while (fgets(line, 1024, fp) != NULL) {
			char* pch = strtok(line, " :()");
			std::vector<char*> tokens;
			while (pch != NULL) {
				tokens.push_back(pch);
				pch = strtok(NULL, " :()");
			}
			// sp_print(tokens[9]);
			if (strcmp(tokens[9], "LISTEN") == 0) {
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

}
