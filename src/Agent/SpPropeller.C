#include "SpPropeller.h"
#include "SpContext.h"
#include "PatchMgr.h"
#include "SpSnippet.h"
#include "Instruction.h"

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
                     PayloadFunc head,
                     PayloadFunc tail) {
  // if (pts.size() == 0) return false;

  // 1. Find points according to type
  Points pts;
  PatchMgrPtr mgr = context->mgr();
  PatchFunction* cur_func = context->parser()->findFunction(func->name());
  next_points(cur_func, mgr, pts);

  sp_debug("PROPELLING - To %d points", pts.size());

  // 2. Start instrumentation
  Dyninst::PatchAPI::Patcher patcher(mgr);
  for (int i = 0; i < pts.size(); i++) {
    Point* pt = pts[i];
    sp_debug("SNIP INSERT - insert snippet");
    PatchFunction* callee = context->parser()->callee(pt);
    if (!callee) sp_debug("INDIRECT CALL - instrumenting indirect call");

    SpSnippet::ptr sp_snip = SpSnippet::create(callee, pt, context, head, tail);
    sp_debug("CREATED - snip insert command");

    Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::create(sp_snip);
    patcher.add(PushBackCommand::create(pt, snip));
    sp_debug("ADDED - snip insert command");
  }
  sp_debug("CODE GEN - starting");
  patcher.commit();
  return true;
}

void SpPropeller::next_points(PatchFunction* cur_func, PatchMgrPtr mgr, Points& pts) {
  //PatchFunction* cur_func = context->parser()->findFunction(func);
  Scope scope(cur_func);
  Points tmp_pts;
  if (!mgr->findPoints(scope, Point::PreCall, back_inserter(tmp_pts))) {
    sp_debug("NO POINT - cannot find any point for function %s", cur_func->name().c_str());
  }
  sp_debug("POINTS - %d in total", tmp_pts.size());
  for (Points::iterator pi = tmp_pts.begin(); pi != tmp_pts.end(); pi++) {
    // if (!context->parser()->callee(*pi)) continue;
    pts.push_back(*pi);
    /*
    sp_debug("%s at %lx (%d pts) - Call %s at Block %lx ~ %lx",
             cur_func->name().c_str(), cur_func->addr(), pts.size(),
             context->parser()->callee(*pi)->name().c_str(),
             (*pi)->block()->start(), (*pi)->block()->end());
    */
  }
}
