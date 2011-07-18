#include "Propeller.h"
#include "Context.h"

using sp::Propeller;
using sp::Payload;
using sp::Context;

Propeller::Propeller() {
  sp_debug("%s", __FUNCTION__);
}

Propeller::ptr Propeller::create() {
  return ptr(new Propeller);
}

bool Propeller::go(Propeller::PointType type,
                   Payload::ptr payload) {
  sp_debug("%s", __FUNCTION__);

  // 0. Restore previous overwritten instruction
  //    should build a map (orig addr -> orig inst)

  // 1. Find points according to type
  //    - mgr = get PatchMgr from context_
  //    - cur_func = get current function from context_
  //    - findPoints(cur_func, type, points)
  context_->getCurrentFunc();
  switch (type) {
    case CALLEE: {
      break;
    }
  }

  // 2. Make snippet according to payload
  //    - payload_addr = get function address for this payload
  //    - build binary blob for calling payload_addr

  // 3. Insert snippet

  return true;
}

void Propeller::set_context(ContextPtr c) {
  sp_debug("%s", __FUNCTION__);
  context_ = c;
}
