#include "SpEvent.h"

using sp::SpEvent;
using sp::NowEvent;
using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;

SpEvent::SpEvent() {
  sp_debug("%s", __FUNCTION__);
}

void SpEvent::register_event(SpContextPtr c) {
  sp_debug("Event::%s", __FUNCTION__);
  // c->propel
  //  - propel(POINT_TYPE, init_payload);
}

NowEvent::NowEvent() {
  sp_debug("%s", __FUNCTION__);
}

void NowEvent::register_event(SpContextPtr c) {
  sp_debug("NowEvent::%s", __FUNCTION__);
  c->propel(SpPropeller::CALLEE, c->init_payload());
}
