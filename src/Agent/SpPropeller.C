#include "SpPropeller.h"
#include "SpContext.h"
#include "PatchMgr.h"
#include "SpSnippet.h"

using sp::SpPropeller;
using sp::SpContext;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::Snippet;

SpPropeller::SpPropeller() {
}

SpPropeller::ptr SpPropeller::create() {
  return ptr(new SpPropeller);
}

bool SpPropeller::go(PatchFunction* func,
                    SpContextPtr context,
                    PayloadFunc payload) {
  // 0. Restore previous overwritten instruction
  //    should build a map (orig addr -> orig inst)
  sp_debug("PROPELLING - To function %s", func->name().c_str());

  // 1. Find points according to type
  std::vector<Point*> pts;
  PatchMgrPtr mgr = context->mgr();
  Scope scope(func);
  if (!mgr->findPoints(scope, Point::PreCall, back_inserter(pts))) {
    sp_debug("NO POINT - cannot find any point for function %s", func->name().c_str());
    return false;
  }
  sp_debug("POINT - %d points found in function %s", pts.size(), func->name().c_str());

  // 2. Start instrumentation
  Dyninst::PatchAPI::Patcher patcher(mgr);
  for (int i = 0; i < pts.size(); i++) {
    Point* pt = pts[i];
    PatchFunction* callee = pt->getCallee();
    SpSnippet::ptr sp_snip = SpSnippet::create(callee, context, payload);
    Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::create(sp_snip);
    patcher.add(PushBackCommand::create(pt, snip));
  }
  patcher.commit();
  return true;
}

