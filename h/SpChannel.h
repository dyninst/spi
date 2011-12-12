#ifndef _SPCHANNEL_H_
#define _SPCHANNEL_H_

#include "SpAgentCommon.h"

namespace sp {

typedef enum {
  SP_UNKNOWN = 0,
  SP_PIPE = 1,
  SP_TCP = 2,
  SP_UDP = 3 
} ChannelType;


// Uni-directional channel
// Local process is the one that sends or writes to the channel
// Remote process is the one that receives or reads from the channel
struct SpChannel {
  SpChannel() :
  type(SP_UNKNOWN), injected(false), local_pid(0), local_ip(0), remote_pid(0), remote_ip(0), remote_injected(false), inode(-1) {}

  ChannelType type;      // Channel type
  bool injected;         // Already injected?
  pid_t local_pid;       // Sender's pid
  unsigned local_ip;     // Sender's ip
  pid_t remote_pid;      // Receiver's pid
  unsigned remote_ip;    // Receiver's ip
  bool remote_injected;  // Agent is already injected to receiver?
  long inode;            // System-wide inode number
};

struct PipeChannel : public SpChannel {
  PipeChannel() : SpChannel() {}
};


struct TcpChannel : public SpChannel {
  TcpChannel() :
  SpChannel(), local_port(0), remote_port(0) {}
  unsigned local_port;
  unsigned remote_port;
};

struct UdpChannel : public SpChannel {
  UdpChannel() :
  SpChannel(), local_port(0), remote_port(0) {}

  unsigned local_port;
  unsigned remote_port;
};

}
#endif /* _SPCHANNEL_H_ */
