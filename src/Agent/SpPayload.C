#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchObject;

void default_head(Point* pt, sp::SpContext* context) {

  PatchFunction* f = context->callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::Points pts;
  sp::CalleePoints(f, context, pts);
  sp_debug("FIND POINTS - %d points found in function %s", pts.size(), callee_name.c_str());
  sp::SpPropeller::ptr p = sp::SpPropeller::create();
  p->go(pts, context, context->init_head(), context->init_tail());
}

void default_tail(Point* pt, sp::SpContext* context) {
  PatchFunction* f = context->callee(pt);
  if (!f) return;
  string callee_name = f->name();
  sp_print("Leave %s", callee_name.c_str());
}
