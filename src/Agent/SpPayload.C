#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
//#include "SpNextPoints.h"
#include "SpUtils.h"

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
  //  sp::payload_start();
  SpPayload payload(pt, context);
  PatchFunction* f = payload.callee();
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  payload.propell();
  //  sp::payload_end();
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

void SpPayload::propell() {

  // Skip if we have already propagated from this point
  SpContext::PropMap& prop_map = context_->prop_map();
  if (prop_map.find(pt_) != prop_map.end()) {
    return;
  }

  PatchFunction* f = callee();
  if (!f) return;

  sp::SpPropeller::ptr p = context_->init_propeller();
  p->go(f, context_, context_->init_head(), context_->init_tail());
  prop_map[pt_] = true;
}

