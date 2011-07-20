#include "SpEvent.h"

#include "signal.h"

using sp::SpEvent;
using sp::NowEvent;
using sp::TimerEvent;

using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;

/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
  sp_debug("%s", __FUNCTION__);
}

void SpEvent::register_event(SpContextPtr c) {
  sp_debug("Event::%s", __FUNCTION__);
}

/* Now Event */
NowEvent::NowEvent() {
  sp_debug("%s", __FUNCTION__);
}

void NowEvent::register_event(SpContextPtr c) {
  sp_debug("NowEvent::%s", __FUNCTION__);
  c->propel(SpPropeller::CALLEE, c->init_payload());
}

/* Timer Event */
sp::SpContextPtr g_context = sp::SpContextPtr();
void alarm_handler(int) {
  assert(g_context);
  g_context->propel(sp::SpPropeller::CALLEE, g_context->init_payload());
}

TimerEvent::TimerEvent(int sec) : after_secs_(sec){
  sp_debug("%s", __FUNCTION__);
}

void TimerEvent::register_event(SpContextPtr c) {
  sp_debug("TimerEvent::%s", __FUNCTION__);
  g_context = c;
  signal(SIGALRM, alarm_handler);
  alarm(after_secs_);
}
