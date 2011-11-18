#include "SpPropeller.h"
#include "SpContext.h"
#include "PatchMgr.h"
#include "SpSnippet.h"
#include "Instruction.h"
#include "SpUtils.h"

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
using Dyninst::PatchAPI::PatchBlock;

SpPropeller::SpPropeller() {
}

SpPropeller::ptr SpPropeller::create() {
  return ptr(new SpPropeller);
}

bool SpPropeller::go(PatchFunction* func,
                     SpContext* context,
                     PayloadFunc before,
                     PayloadFunc after,
                     Point* pt) {

  // 1. Find points according to type
  Points pts;
  PatchMgrPtr mgr = context->mgr();
  PatchFunction* cur_func = NULL;
  if (pt) {
    cur_func = func;
  } else {
    // sp_print("cur_func: %s", func->name().c_str());
    cur_func = context->parser()->findFunction(func->name());
  }
  next_points(cur_func, mgr, pts);

  // 2. Start instrumentation
  Dyninst::PatchAPI::Patcher patcher(mgr);
  for (int i = 0; i < pts.size(); i++) {
    Point* pt = pts[i];

    // It's possible that callee will be NULL, which is an indirect call.
    // In this case, we'll parse it later during runtime.
    PatchFunction* callee = context->parser()->callee(pt);
    SpSnippet::ptr sp_snip = SpSnippet::create(callee, pt, context, before, after);
    Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::create(sp_snip);
    patcher.add(PushBackCommand::create(pt, snip));
  }
  patcher.commit();
  return true;
}

void SpPropeller::next_points(PatchFunction* cur_func, PatchMgrPtr mgr, Points& pts) {
  Scope scope(cur_func);
  //  Points tmp_pts;
  mgr->findPoints(scope, Point::PreCall, back_inserter(pts));
  /*
  for (Points::iterator pi = tmp_pts.begin(); pi != tmp_pts.end(); pi++) {
    pts.push_back(*pi);
  }
  */
}
