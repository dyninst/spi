#include "Event.h"

using sp::Event;
using sp::NowEvent;
using sp::Context;
using sp::ContextPtr;
using sp::Propeller;

Event::Event() {
  sp_debug("%s", __FUNCTION__);
}

void Event::register_event(ContextPtr c) {
  sp_debug("Event::%s", __FUNCTION__);
  // c->propel
  //  - propel(POINT_TYPE, init_payload);
}

NowEvent::NowEvent() {
  sp_debug("%s", __FUNCTION__);
}

void NowEvent::register_event(ContextPtr c) {
  sp_debug("NowEvent::%s", __FUNCTION__);
  c->propel(Propeller::CALLEE, c->init_payload());
}
