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

        if (sscanf(buffer, "pipe:[%lu]", &temp_node) == 1 &&
            temp_node == inode) {
          closedir(dir);
          return true;
        } // Anonymous pipe

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

	static int proto_addr_to_inode(FILE *fproto,
																 in_addr_t loc_ip, uint16_t loc_port,
																 in_addr_t rem_ip, uint16_t rem_port)
	{
    char buffer[MAXLEN];
    unsigned iloc_ip, iloc_port, irem_ip, irem_port;
    unsigned inode;

    if (fgets(buffer, MAXLEN, fproto) == 0) { /* skip the header line */
			perror("addr_to_inode: fgets error");
			return (-1);
    }
    while (fgets(buffer, MAXLEN, fproto) != 0) {
			sp_print("%s", buffer);
			sp_print("%08X:%08X", loc_ip, loc_port);
			if (sscanf(buffer, "%*u: %08X:%04X %08X:%04X %*02X %*08X:%*08X "
								 "%*02X:%*08X %*08X %*u %*d %u",
								 &iloc_ip, &iloc_port, &irem_ip, &irem_port, &inode) != 5) {
				fprintf(stderr, "addr_to_inode: file format error at line \"%s\"",
								buffer);
				return (-1);
			}
			if ((iloc_ip == loc_ip || iloc_ip == 0 /* any */) &&
      iloc_port == loc_port &&
					irem_ip == rem_ip && irem_port == rem_port) {
				return inode;
			}
    }
    return 0;
	}

	static ino_t addr_to_inode(in_addr_t loc_ip, uint16_t loc_port,
							  						 in_addr_t rem_ip, uint16_t rem_port)	{   
    FILE *ftcp, *fudp;
    int inode;

    if ((ftcp = fopen("/proc/net/tcp", "rt")) == 0) {
			perror("addr_to_inode: fopen(tcp) error");
			return 0;
    }
    inode = proto_addr_to_inode(ftcp, loc_ip, loc_port, rem_ip, rem_port);
    fclose(ftcp);

    if (inode < 0) {
			return 0;
    }
    else if (inode != 0) {
			return inode;
    }

    if ((fudp = fopen("/proc/net/udp", "rt")) == 0) {
			perror("addr_to_inode: fopen(udp) error");
			return 0;
    }
    inode = proto_addr_to_inode(fudp, loc_ip, loc_port, rem_ip, rem_port);
    fclose(fudp);

    if (inode < 0) {
			return 0;
    }
    else if (inode != 0) {
			return inode;
    }
    fprintf(stderr, "addr_to_inode: addr 0x%x:0x%x:0x%x:0x%x not found\n",
						loc_ip, loc_port, rem_ip, rem_port);

    return 0;
	}
#if 0
	static int proto_inode_to_addr(FILE *fproto, unsigned inode,
																 unsigned *loc_ip, unsigned *loc_port,
																 unsigned *rem_ip, unsigned *rem_port)
	{   
    char buffer[MAXLEN];
    unsigned iloc_ip, iloc_port, irem_ip, irem_port, iinode;
  
    if (fgets(buffer, MAXLEN, fproto) == 0) { /* skip the header line */
			perror("inode_to_addr: fgets error");
			return (-1);
    }
    while (fgets(buffer, MAXLEN, fproto) != 0) {
			if (sscanf(buffer, "%*u: %08X:%04X %08X:%04X %*02X %*08X:%*08X "
								 "%*02X:%*08X %*08X %*u %*d %u",
								 &iloc_ip, &iloc_port, &irem_ip, &irem_port, &iinode) != 5) {
				fprintf(stderr, "inode_to_addr: file format error at line \"%s\"",
								buffer);
				return (-1);
			}
			if (iinode == inode) {
				*loc_ip = iloc_ip;
				*loc_port = iloc_port;
				*rem_ip = irem_ip;
				*rem_port = irem_port;
				return 1;
			}
    }
    return 0; /* not found */
	}

	int inode_to_addr(unsigned inode,
										unsigned *loc_ip, unsigned *loc_port,
										unsigned *rem_ip, unsigned *rem_port)
	{   
    FILE *ftcp, *fudp;
    int rv;

    if ((ftcp = fopen("/proc/net/tcp", "rt")) == 0) {
			perror("inode_to_addr: fopen error");
			return (-1);
    }
    rv = proto_inode_to_addr(ftcp, inode, loc_ip, loc_port, rem_ip, rem_port);
    fclose(ftcp);

    if (rv < 0) {
			return (-1);
    }
    else if (rv != 0) {
			return 0;
    }
    if ((fudp = fopen("/proc/net/udp", "rt")) == 0) {
			perror("inode_to_addr: fopen(udp) error");
			return (-1);
    }
    rv = proto_inode_to_addr(fudp, inode, loc_ip, loc_port, rem_ip, rem_port);
    fclose(fudp);

    if (rv < 0) {
			return (-1);
    }
    else if (rv != 0) {
			return 0;
    }
    fprintf(stderr, "inode_to_addr: inode %u not found\n", inode);
    return (-1);
	}
#endif

	static int inode_to_pids(ino_t inode, PidSet& pid_set)	{
#if 0
    int pid, rv, num_found = 0;
    DIR *dir;
    char *ep;
    struct dirent *de;

    if ((dir = opendir("/proc")) == 0) {
			perror("inode_to_pids: opendir(/proc) error");
			return (-1);
    }

    errno = 0; /* To distinguish eof from error */
    while ((de = readdir(dir)) != 0) {
			if (isdigit(de->d_name[0])) {
				pid = strtol(de->d_name, &ep, 10);
				if (ep == 0 || *ep != 0 || pid < 0) {
					fprintf(stderr, "inode_to_pids: strtol failed on %s\n",
									de->d_name);
					return (-1);
				}
				if ((rv = pid_uses_inode(pid, inode)) < 0) {
					return (-1);
				}
				else if (rv != 0) {
					if (num_found >= *pnum_pids) {
						fprintf(stderr, "inode_to_pids: not enough space\n");
						return (-1);
					}
					pids[num_found] = pid;
					num_found++;
				}
			}
    }
    if (errno != 0) {
			perror("inode_to_pids: readdir error");
			return (-1);
    }
    closedir(dir);
    *pnum_pids = num_found;
#endif
    return 0;
	}

	// Get pids that are associated with the local/remote address pair
	void addr_to_pids(in_addr_t loc_ip, uint16_t loc_port,
										in_addr_t rem_ip, uint16_t rem_port,
										PidSet& pid_set) {
    ino_t inode = 0;
    unsigned num_tries = 5;
    while (num_tries != 0) {
			if ((inode = addr_to_inode(loc_ip, loc_port, rem_ip, rem_port)) == 0) {
				sp_print("inode: %lu", inode);
				return;
			}
			else if (inode > 0) {
				sp_print("inode: %lu", inode);
				inode_to_pids(inode, pid_set);
				return;
			}
			else {
        // /proc/net/tcp seems to lag behind
				sp_print("retry");
				sp_print("addr_to_pids: stale /proc/net/tcp, retrying\n");
				sleep(1);
				num_tries--;
			}
    }
    sp_print("addr_to_pids: inode for 0x%x:0x%x:0x%x:0x%x is 0\n",
						loc_ip, loc_port, rem_ip, rem_port);
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
