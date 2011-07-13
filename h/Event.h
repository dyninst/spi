#ifndef SP_EVENT_H_
#define SP_EVENT_H_

#include "Common.h"

namespace sp {
class Event {
  public:
    typedef dyn_detail::boost::shared_ptr<Event> ptr;
    static ptr create() { return ptr(new Event); }
    virtual void register_event() {}
};

/* Instrument current function's callees */
class NowEvent : public Event {
  public:
    typedef dyn_detail::boost::shared_ptr<NowEvent> ptr;
    static ptr create() { return ptr(new NowEvent); }
    virtual void register_event();
};

}


#endif /* SP_EVENT_H_ */
