#ifndef _SPIPCMGR_H_
#define _SPIPCMGR_H_

#include "SpAgentCommon.h"
#include "SpChannel.h"

namespace sp {

class SpIpcMgr {
  public:
    SpIpcMgr();
    ~SpIpcMgr();

    // A payload_before function does two things:
    // 1. work: user-defined logic
    // 2. propagate: propagate instrumentation
    // Here we let user determine if it is okay to work 
    // char can_work();
    // void set_work(char b, int pid);

    // Get channel from fd
    // If channel doesn't exist, construct one
    SpChannel* get_channel(int fd);

    bool is_sender(const char* f);
    bool is_receiver(const char* f);
    bool is_pipe(int fd);
    bool is_tcp(int fd) { return false;}
    bool is_udp(int fd) { return false;}
    bool is_ipc(int fd);
    bool is_fork(const char* f);

    char start_tracing();
    void set_start_tracing(char t, pid_t pid);
  protected:
    char* can_work_;

    // inode -> SpChannel
    typedef std::map<long, SpChannel*> ChannelMap;
    ChannelMap channel_map_;
    typedef std::set<pid_t> PidSet;
    PidSet child_proc_set_;
    char* start_tracing_;

    long get_inode_from_fd(int fd);
    void get_pids_from_fd(int fd, PidSet& pid_set);

    void destroy_channel(SpChannel* c);
};

}

#endif /* _SPIPCMGR_H_ */
