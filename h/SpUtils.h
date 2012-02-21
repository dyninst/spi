#ifndef _SPUTILS_H_
#define _SPUTILS_H_

#include <set>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>

#include "SpCommon.h"

namespace sp {
	typedef std::set<pid_t> PidSet;
  typedef std::set<std::string> StringSet;
  
  // Profiling tools (e.g., timer)
  void SetupTimer(int timer_id);
  void StartTimer(int timer_id);
  void StopTimer(int timer_id);
  void ResetTimer(int timer_id);
  double GetTimer(int timer_id);
  void PrintTime(char* message, int timer_id);


  // Code Generation stuffs
  bool IsDisp32(long d);                     // Is 32-bit displacement?
  bool IsDisp8(long d);                      // Is 8-bit displacement?
  bool IsPcRegister(dt::MachRegister reg);   // Is a pc register?


  // Get pid from various things

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


// -------------------------------------------------------------------
// /proc utilities
// -------------------------------------------------------------------
		// Is current executable an illegal program?
		// We use this, because we want to avoid instrumenting some programs, e.g.,
		// unix utilities used in self-propelled core
		bool is_illegal_exe(StringSet& illegal_exes);

		// Get text content from a file. If file doesn't exist, return "".
		std::string get_file_text(std::string filename);

		// Get full path of the executable file
		std::string get_exe_name();
// -------------------------------------------------------------------
// IPC stuffs
// -------------------------------------------------------------------

		// See if the file descriptor is for pipe
    bool is_pipe(int fd);

		// See if the file descriptor is for tcp
    bool is_tcp(int fd);

		// See if the file descriptor is for udp
    bool is_udp(int fd);

		// See if the file descriptor is for any ipc mechanism
    bool is_ipc(int fd);

// ---------------------------------------------------------------------
// Socket programming things
// ---------------------------------------------------------------------
	bool get_address(sockaddr_storage* sa, char* host, size_t host_len,
									 char* service, size_t service_len);
	bool get_local_address(int fd, sockaddr_storage* out);
	bool get_remote_address(int fd, sockaddr_storage* out);

// ---------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------
	bool serialize_co(pe::CodeObject* co, const char* dir);
	pe::CodeObject* deserialize_co(const char* dir, const char* file);

// ------------------------------------------------------------------- 
// Lock / Unlock
// -------------------------------------------------------------------
	void init_lock(int* lock);
	void lock(int* lock);
	void unlock(int* lock);

}

#endif /* _SPUTILS_H_ */
