#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using sp::SpPayload;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchObject;

//-----------------------------------------
// Default payload functions
//-----------------------------------------
void default_head(Point* pt, sp::SpContext* context) {
  SpPayload payload(pt, context);
  PatchFunction* f = payload.callee();
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  payload.default_propell();
}

void default_tail(Point* pt, sp::SpContext* context) {
  SpPayload payload(pt, context);
  PatchFunction* f = payload.callee();
  if (!f) return;

  string callee_name = f->name();
  sp_print("Leave %s", callee_name.c_str());
}

//-----------------------------------------
// SpPayload
//-----------------------------------------
SpPayload::SpPayload(Dyninst::PatchAPI::Point* pt, SpContext* context)
  : pt_(pt), context_(context) {
  context_->callee(pt_);
}

Dyninst::PatchAPI::PatchFunction* SpPayload::callee() {
  return context_->callee(pt_);
}

void SpPayload::default_propell() {
  PatchFunction* f = callee();
  if (!f) return;

  sp::Points pts;
  sp::CalleePoints(f, context_, pts);
  sp::SpPropeller::ptr p = context_->init_propeller();
  p->go(pts, context_, context_->init_head(), context_->init_tail());
}
