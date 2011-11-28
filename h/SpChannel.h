#ifndef _SPCHANNEL_H_
#define _SPCHANNEL_H_

namespace sp {

typedef enum {
  SP_UNKNOWN = 0;
  SP_PIPE = 1;
  SP_TCP = 2;
  SP_UDP = 3; 
} ChannelType;

struct SpChannel {
  SpChannel() :
  id(0), type(SP_UNKNOWN), propagated(false), sender_pid(0), receiver_pid(0) {}

  long id;               // System-wide id
  ChannelType type;      // Channel type
  bool propagated;       // Already propagated?
  pid_t local_pid;       // Sender's pid
  pid_t remote_pid;      // Receiver's pid
  int fd;                // File descriptor
};

struct SpPipeChannel : public SpChannel {
  SpPipeChannel() : SpChannel() {}
};

struct SpTcpChannel : public SpChannel {
  SpTcpChannel() :
  SpChannel(), local_ip(0), local_port(0), remote_ip(0), remote_port(0) {}

  unsigned local_ip;
  unsigned local_port;
  unsigned remote_ip;
  unsigned remote_port;
};

struct SpUdpChannel : public SpChannel {
  SpUdpChannel() :
  SpChannel(), local_ip(0), local_port(0), remote_ip(0), remote_port(0) {}

  unsigned local_ip;
  unsigned local_port;
  unsigned remote_ip;
  unsigned remote_port;
};

}
#endif /* _SPCHANNEL_H_ */
