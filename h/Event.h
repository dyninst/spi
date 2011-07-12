#ifndef SP_EVENT_H_
#define SP_EVENT_H_

#include "Common.h"

namespace sp {
class Event {
  public:
    typedef dyn_detail::boost::shared_ptr<Event> ptr;
    virtual void register_event() {}
};

class NowEvent : public Event {
  public:
    virtual void register_event();
};

}


#endif /* SP_EVENT_H_ */
