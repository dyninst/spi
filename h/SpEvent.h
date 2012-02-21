#ifndef SP_EVENT_H_
#define SP_EVENT_H_

#include <signal.h>

#include "SpCommon.h"
#include "SpPropeller.h"

namespace sp {

class SpEvent {
  public:
    virtual ~SpEvent() {}

    typedef dyn_detail::boost::shared_ptr<SpEvent> ptr;
    static ptr create() { return ptr(new SpEvent); }
    virtual void register_event();
  protected:
    SpEvent();
};


// Instrument current function's callees after N seconds
class AsyncEvent : public SpEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<AsyncEvent> ptr;

    // Parameter "sec" is the time for alarm signal. For other signals, this
		// parameter is ignored
    static ptr create(int signum = SIGALRM, int sec = 5) {
      return ptr(new AsyncEvent(signum, sec)); }
    virtual void register_event();
  protected:
    AsyncEvent(int signum, int sec);
    void* handler_;
    int after_secs_;
    int signum_;
};


// Instrument current function's callees right away
class SyncEvent : public SpEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<SyncEvent> ptr;
    static ptr create(std::string func_name="") {
      return ptr(new SyncEvent(func_name)); }
    void register_event();
  protected:
    SyncEvent(std::string func_name);

    std::string func_name_;
};

}


#endif /* SP_EVENT_H_ */
