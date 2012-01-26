#ifndef _SPUTILS_H_
#define _SPUTILS_H_

#include "SpCommon.h"

// Shorten namespace
namespace dt = Dyninst;

namespace sp {

// ----------------------------------------------------------------------------- 
// Profiling tools
// -----------------------------------------------------------------------------

  void SetupTimer(int);
  void StartTimer(int);
  void StopTimer(int);
  void ResetTimer(int);
  double GetTimer(int);
  void PrintTime(char *, int);

// ----------------------------------------------------------------------------- 
// Code Generation stuffs
// -----------------------------------------------------------------------------

  // Determine whether a long integer has a value within 32-bit range.
  bool is_disp32(long d);

  // Determine whether a long integer has a value within 8-bit range.
  bool is_disp8(long d);

	// Determine whether a register is a pc register (rip for x86-64, and eip for
  // i386)
  bool is_pc(dt::MachRegister);

// -----------------------------------------------------------------------------
// Get pid from various things
// -----------------------------------------------------------------------------

		// Get inode number from the file descriptor fd
    ino_t get_inode_from_fd(int fd);

		// Is this process using this inode?
    bool pid_uses_inode(int pid, ino_t inode);

		// Get all pids that are using this fd
    void get_pids_from_fd(int fd, PidSet& pid_set);

    // Get pids that are associated with the local/remote address pair
    void addr_to_pids(char* loc_ip, char* loc_port,
		                  char* rem_ip, char* rem_port,
                      PidSet& pid_set);

		// Get ip from hostname
		in_addr_t hostname_to_ip(char * hostname , char* ip, size_t ip_len);


// ----------------------------------------------------------------------------- 
// /proc utilities
// -----------------------------------------------------------------------------
		// Is current executable an illegal program?
		// We use this, because we want to avoid instrumenting some programs, e.g.,
		// unix utilities used in self-propelled core
		bool is_illegal_exe(StringSet& illegal_exes);

		// Get text content from a file. If file doesn't exist, return "".
		std::string get_file_text(std::string filename);

// ----------------------------------------------------------------------------- 
// IPC stuffs
// -----------------------------------------------------------------------------

		// See if the file descriptor is for pipe
    bool is_pipe(int fd);

		// See if the file descriptor is for tcp
    bool is_tcp(int fd);

		// See if the file descriptor is for udp
    bool is_udp(int fd);

		// See if the file descriptor is for any ipc mechanism
    bool is_ipc(int fd);

// ----------------------------------------------------------------------------- 
// Socket programming things
// -----------------------------------------------------------------------------
	bool get_address(sockaddr_storage* sa, char* host, size_t host_len,
									 char* service, size_t service_len);
	bool get_local_address(int fd, sockaddr_storage* out);
	bool get_remote_address(int fd, sockaddr_storage* out);
}

#endif /* _SPUTILS_H_ */
