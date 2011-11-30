#ifndef _SPIPCMGR_H_
#define _SPIPCMGR_H_

namespace sp {

class SpIpcMgr {
  public:
    SpIpcMgr();

    // A payload_before function does two things:
    // 1. work: user-defined logic
    // 2. propagate: propagate instrumentation
    // Here we let user determine if it is okay to work 
    char can_work();
    void set_work(char b, int pid);

    bool is_pipe(int fd);
    bool is_tcp(int fd) { return false;}
    bool is_udp(int fd) { return false;} 
  protected:
    char* can_work_;    
};

}

#endif /* _SPIPCMGR_H_ */
