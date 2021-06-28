#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <ctype.h>
#include <sys/dir.h>
#include <unistd.h>
#include <sys/stat.h>

typedef std::set<pid_t> PidSet;

bool
PidUsesInode(const int pid,
             const ino_t inode) {
  DIR *dir;
  ino_t temp_node;
  struct dirent *de;
  char name[1024];
  char buffer[1024];
  snprintf(name, 1024, "/proc/%u/fd", pid);

  if ((dir = opendir(name)) == 0) {
    return false;
  }

  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      snprintf(name, 1024, "/proc/%u/fd/%s", pid, de->d_name);
      int size = -1;
      if ((size = readlink(name, buffer, 1024)) < 0) {
        perror("PidUsesInode: readlink error");
        return false;
      }
      buffer[size] = '\0';

      if (sscanf(buffer, "pipe:[%lu]", &temp_node) == 1 &&
          temp_node == inode) {
        // Anonymous pipe
        closedir(dir);
        return true;
      } else if (sscanf(buffer, "socket:[%lu]", &temp_node) == 1 &&
                 temp_node == inode) {
        // Tcp
        closedir(dir);
        return true;
      } else {
        // Named pipe
        struct stat s;
        if (stat(buffer, &s) != -1) {
          if (s.st_ino == inode) {
            closedir(dir);
            return true;
          }
        }
      }   // Anoymous pipe? tcp? named pipe?
    }   // A process?
  }   // Iterate all processes
  closedir(dir);
  return false;
}

void
GetPidsFromAddrs(const char* const rem_ip,
                 const char* const rem_port,
                 PidSet& pid_set) {

  // Convert port string to int
  int rem_port_int = atoi(rem_port);

  // Read /proc/net/tcp for
  FILE* tcp_fp = fopen("/proc/net/tcp", "r");
  char line[2048];
  int inode = -1;
  if (fgets(line, 2048, tcp_fp) == NULL) {
    perror("Failed to read headline of /proc/net/tcp");
    exit(1);
  }
  while (fgets(line, 2048, tcp_fp) != NULL) {
    int iloc_ip, iloc_port, irem_ip, irem_port;
    if (sscanf(line, "%*u: %08X:%04X %08X:%04X %*02X %*08X:%*08X "
               "%*02X:%*08X %*08X %*u %*d %u",
               &iloc_ip, &iloc_port, &irem_ip, &irem_port, &inode) != 5) {
      perror("Failed to read a line in /proc/net/tcp");
      exit(1);
    }
    if (iloc_port == rem_port_int) break;
  }
  fclose(tcp_fp);
  if (inode == -1) return;

  // Iterate all pids
  DIR *dir;
  int pid;
  if ((dir = opendir("/proc")) == 0) {
    perror("ERROR: cannot access /proc");
    exit(1);
  }
  struct dirent *de;
  char *ep;
  while ((de = readdir(dir)) != 0) {
    if (isdigit(de->d_name[0])) {
      pid = strtol(de->d_name, &ep, 10);
      if (ep == 0 || *ep != 0 || pid < 0) {
        printf("ERROR: strtol failed on %s\n", de->d_name);
        exit(1);
      }
      if (pid != getpid() &&
          PidUsesInode(pid, inode)) {
        pid_set.insert(pid);
        // sp_print("Found pid=%d opening port %d", pid, rem_port_int);
      }
    } // Is pid?
  } // Iterate all pids
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s port\n", argv[0]);
    return -1;
  }
  PidSet pid_set;
  GetPidsFromAddrs("0.0.0.0",
                   argv[1],
                   pid_set);
  if (pid_set.size() > 0)
    fprintf(stdout, "%d", *pid_set.begin());
  return 0;
}
