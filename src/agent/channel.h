#ifndef _SPCHANNEL_H_
#define _SPCHANNEL_H_

#include <sys/socket.h>

#include "common/common.h"

namespace sp {

typedef enum {
  SP_UNKNOWN = 0,
  SP_PIPE = 1,
  SP_TCP = 2,
  SP_UDP = 3 
} ChannelType;

typedef enum {
  SP_READ,
  SP_WRITE,
  SP_NORW
} ChannelRW;

// ----------------------------------------------------------------------------- 
// Uni-directional channel
// Local process is the one that sends or writes to the channel
// Remote process is the one that receives or reads from the channel
// -----------------------------------------------------------------------------
struct SpChannel {
  SpChannel() :
  type(SP_UNKNOWN), injected(false), local_pid(-1), remote_pid(-1),
		remote_injected(false), inode(-1), fd(-1), rw(SP_WRITE) {}

  ChannelType type;      // Channel type
  bool injected;         // Already injected?
  pid_t local_pid;       // Sender's pid
  pid_t remote_pid;      // Receiver's pid
  bool remote_injected;  // Agent is already injected to receiver?
  ino_t inode;           // System-wide inode number
  int fd;                // local fd that represents this channel
  ChannelRW rw;          // Read or Write?
};


// ----------------------------------------------------------------------------- 
// Assume only two parties are involved in a pipe, which is not true in real
// world. For instance, named pipe would have multiple processes involved. 
// Need to consider it later.
// -----------------------------------------------------------------------------
struct PipeChannel : public SpChannel {
  PipeChannel() : SpChannel() {}
};


struct TcpChannel : public SpChannel {
  TcpChannel() :
  SpChannel() {}
	sockaddr_storage local;
	sockaddr_storage remote;
};

struct UdpChannel : public SpChannel {
	/*
  UdpChannel() :
  SpChannel(), local_ip(0), local_port(0), remote_ip(0), remote_port(0) {}

  in_addr_t local_ip;
  uint16_t local_port;
  in_addr_t remote_ip;
  uint16_t remote_port;
	*/
};

}
#endif /* _SPCHANNEL_H_ */
