#ifndef _SPIPCMGR_H_
#define _SPIPCMGR_H_

#include "SpAgentCommon.h"
#include "SpChannel.h"

namespace sp {

class SpPoint;
class SpIpcWorker;
class SpPipeWorker;
class SpUdpWorker;
class SpTcpWorker;

class SpIpcMgr {
  public:
    SpIpcMgr();
    ~SpIpcMgr();

    /* Get channel from fd
       If channel doesn't exist, construct one
    */
    SpChannel* get_channel(int fd, ChannelRW rw);
    SpIpcWorker* get_worker(int fd);

    void get_write_param(SpPoint* pt, int* fd_out, void** buf_out,
                         char* c_out, size_t* size_out);

    void get_read_param(SpPoint* pt, int* fd_out, void** buf_out,
                        size_t* size_out);

    bool is_pipe(int fd);
    bool is_tcp(int fd);
    bool is_udp(int fd);
    bool is_ipc(int fd);

    bool is_fork(const char* f);
    bool is_popen(const char* f);

    char start_tracing();

    static bool pre_before(SpPoint*);
    static bool pre_after(SpPoint*);

    SpPipeWorker* pipe_worker() const { return pipe_worker_; }
    SpTcpWorker* tcp_worker() const { return tcp_worker_; }
    SpUdpWorker* udp_worker() const { return udp_worker_; }

  protected:

    /* IPC workers */
    SpPipeWorker* pipe_worker_;
    SpTcpWorker* tcp_worker_;
    SpUdpWorker* udp_worker_;

    typedef std::set<SpIpcWorker*> WorkerSet;
    WorkerSet worker_set_;
};

/* IPC workers */
class SpIpcWorker {
  public:
    /* A payload_before function does two things:
       1. tracing: user-defined logic
       2. propagate: propagate instrumentation
       Here we let user determine if it is okay to trace
    */
    virtual void set_start_tracing(char yes_or_no, pid_t) = 0;
    virtual char start_tracing() = 0;
    virtual bool inject(SpChannel*) = 0;

    /* Get IPC channel from a file descriptor. 
       Return: NULL if not a valid IPC channel; otherwise, the channel.
    */
    virtual SpChannel* get_channel(int fd, ChannelRW rw) = 0;
};

class SpPipeWorker : public SpIpcWorker {
  public:
    SpPipeWorker();
    ~SpPipeWorker();

    virtual void set_start_tracing(char yes_or_no, pid_t);
    virtual char start_tracing();
    virtual bool inject(SpChannel*);
    virtual SpChannel* get_channel(int fd, ChannelRW rw);

  protected:
    /* inode-to-SpChannel mapping */
    typedef std::map<long, SpChannel*> ChannelMap;
    ChannelMap channel_map_write_;
    ChannelMap channel_map_read_;

    /* Child process set */
    typedef std::set<pid_t> PidSet;
    PidSet child_proc_set_;

    /* Can payload do trace? */
    char* start_tracing_;

    void tracing_internal(char** start_tracing);
    int pid_uses_inode(int pid, int inode);
    long get_inode_from_fd(int fd);
    void get_pids_from_fd(int fd, PidSet& pid_set);
};


class SpTcpWorker : public SpIpcWorker {
  public:
    virtual void set_start_tracing(char yes_or_no, pid_t);
    virtual char start_tracing();
    virtual bool inject(SpChannel*);
    virtual SpChannel* get_channel(int fd, ChannelRW rw);

};


class SpUdpWorker : public SpIpcWorker {
  public:
    virtual void set_start_tracing(char yes_or_no, pid_t);
    virtual char start_tracing();
    virtual bool inject(SpChannel*);
    virtual SpChannel* get_channel(int fd, ChannelRW rw);

};

}

#endif /* _SPIPCMGR_H_ */
