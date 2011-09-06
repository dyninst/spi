#ifndef SP_EVENT_H_
#define SP_EVENT_H_

#include <signal.h>
#include <string>

#include "SpCommon.h"
#include "SpPropeller.h"

namespace sp {
class SpContext;
class SpEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<SpEvent> ptr;
    static ptr create() { return ptr(new SpEvent); }
    virtual void register_event(SpContext*);
  protected:
    SpEvent();
};


/* Instrument current function's callees after N seconds */
class AsyncEvent : public SpEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<AsyncEvent> ptr;
    /* Parameter "sec" is the time for alarm signal. For other signals, this
       parameter is ignored*/
    static ptr create(int signum = SIGALRM, int sec = 5) {
      return ptr(new AsyncEvent(signum, sec)); }
    virtual void register_event(SpContext*);
  protected:
    AsyncEvent(int signum, int sec);
    void* handler_;
    int after_secs_;
    int signum_;
};


/* Instrument current function's callees right away
   For now, the synchronous event is not really "synchronous".
   It is implemented as a syncEvent of alarm signal with 2 seconds
 */
class SyncEvent : public AsyncEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<SyncEvent> ptr;
    static ptr create(std::string func_name="", int sec=2) {
      return ptr(new SyncEvent(func_name, sec)); }
    void register_event(SpContext* c);
  protected:
    SyncEvent(std::string func_name, int sec);

    std::string func_name_;
};

}


#endif /* SP_EVENT_H_ */
