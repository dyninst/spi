#include "SpNextPoints.h"
#include "SpContext.h"

#include "PatchMgr.h"

using sp::Points;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::Point;

void sp::CalleePoints(PatchFunction* func, SpContext* context, Points& pts) {
  PatchMgrPtr mgr = context->mgr();
  PatchFunction* cur_func = context->parser()->findFunction(func->name());
  Scope scope(cur_func);
  Points tmp_pts;
  if (!mgr->findPoints(scope, Point::PreCall, back_inserter(tmp_pts))) {
    sp_debug("NO POINT - cannot find any point for function %s", func->name().c_str());
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
