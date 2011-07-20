#include "SpContext.h"
#include "SpPropeller.h"
#include "PatchMgr.h"
#include "PatchCFG.h"
#include "frame.h"
#include "walker.h"

using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;
using sp::SpPayload;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::Stackwalker::Walker;
using Dyninst::Stackwalker::Frame;

SpContext::SpContext(SpPropeller::ptr p,
                     SpPayload::ptr ip,
                     PatchMgrPtr mgr) {
  sp_debug("%s", __FUNCTION__);
  assert(p);
  assert(ip);

  propeller_ = p;
  init_payload_ = ip;
  mgr_ = mgr;
}

SpContextPtr SpContext::create(SpPropeller::ptr propeller,
                               SpPayload::ptr init_payload,
                               PatchMgrPtr mgr) {
  sp_debug("SpContext::%s", __FUNCTION__);
  SpContextPtr ret = SpContextPtr(new SpContext(propeller,
                                                init_payload,
                                                mgr));
  assert(ret);
  propeller->set_context(ret);
  return ret;
}

bool SpContext::propel(SpPropeller::PointType type,
                       SpPayload::ptr payload) {
  sp_debug("%s", __FUNCTION__);
  propeller_->go(type, payload);
}

PatchFunction* SpContext::getCurrentFunc() {
  PatchFunction* func = NULL;

  std::vector<Frame> stackwalk;
  string s;
  Walker *walker = Walker::newWalker();
  walker->walkStack(stackwalk);
  for (unsigned i=0; i<stackwalk.size(); i++) {
    stackwalk[i].getName(s);
    sp_debug("FuncCall: %s", s.c_str());
  }

  return func;
}
