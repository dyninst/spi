#include "SpPropeller.h"
#include "SpContext.h"

using sp::SpPropeller;
using sp::SpPayload;
using sp::SpContext;

SpPropeller::SpPropeller() {
  sp_debug("%s", __FUNCTION__);
}

SpPropeller::ptr SpPropeller::create() {
  return ptr(new SpPropeller);
}

bool SpPropeller::go(SpPropeller::PointType type,
                     SpPayload::ptr payload) {
  sp_debug("%s", __FUNCTION__);

  // 0. Restore previous overwritten instruction
  //    should build a map (orig addr -> orig inst)

  // 1. Find points according to type
  //    - mgr = get PatchMgr from context_
  //    - cur_func = get current function from context_
  //    - findPoints(cur_func, type, points)
  //context_->getCurrentFunc();
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

void SpPropeller::set_context(SpContextPtr c) {
  sp_debug("%s", __FUNCTION__);
  context_ = c;
}
