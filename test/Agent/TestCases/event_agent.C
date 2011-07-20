#include <signal.h>

#include "test_case.h"
#include "SpAgent.h"

using sp::SpParser;
using sp::SpAgent;


class MyEvent : public sp::SpEvent {
  public:
    typedef dyn_detail::boost::shared_ptr<MyEvent> ptr;
    static ptr create() { return ptr(new MyEvent); }
    virtual void register_event(sp::SpContextPtr);
  protected:
    MyEvent() {}
};

sp::SpContextPtr g_c;
void alarm_handler(int) {
  g_c->propel(sp::SpPropeller::CALLEE, g_c->init_payload());
}

void MyEvent::register_event(sp::SpContextPtr c) {
  sp_debug("NowEvent::%s", __FUNCTION__);
  g_c = c;
  signal(SIGALRM, alarm_handler);
  alarm(5);
}

void exitme(void) {
  dprint("AGENT: exit me!");
}

AGENT_INIT
void init_event() {
  dprint("AGENT: init_event @ process %d", getpid());

  SpAgent::ptr agent = SpAgent::create();
  agent->set_init_event(MyEvent::create());
  agent->go();

  atexit(exitme);
}
