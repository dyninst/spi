#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpPoint.h"
#include "SpUtils.h"

//using sp::SpPayload;
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
void default_head(Point* pt) {
  //  sp::payload_start();
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::propel(pt);
  //  sp::payload_end();
}

void default_tail(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Leave %s", callee_name.c_str());
}

//-----------------------------------------
// SpPayload
//-----------------------------------------
extern sp::SpContext* g_context;
Dyninst::PatchAPI::PatchFunction* sp::callee(Dyninst::PatchAPI::Point* pt_) {
  return g_context->callee(pt_);
}

void sp::propel(Dyninst::PatchAPI::Point* pt_) {
  // Skip if we have already propagated from this point
  sp::SpPoint* spt = static_cast<sp::SpPoint*>(pt_);
  if (spt->propagated()) {
    return;
  }

  PatchFunction* f = callee(pt_);
  if (!f) return;

  sp::SpPropeller::ptr p = g_context->init_propeller();
  p->go(f, g_context, g_context->init_head(), g_context->init_tail(), pt_);
  spt->set_propagated(true);
}

