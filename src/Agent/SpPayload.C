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
namespace sp {
Dyninst::PatchAPI::PatchFunction* callee(Dyninst::PatchAPI::Point* pt_) {
  return g_context->callee(pt_);
}

void* pop_argument(Dyninst::PatchAPI::Point* pt, ArgumentHandle* h, size_t size) {
  return static_cast<SpPoint*>(pt)->snip()->pop_argument(h, size);
}

void propel(Dyninst::PatchAPI::Point* pt_) {
  // Skip if we have already propagated from this point
  SpPoint* spt = static_cast<sp::SpPoint*>(pt_);
  if (spt->propagated()) {
    return;
  }

  PatchFunction* f = callee(pt_);
  if (!f) return;

  sp::SpPropeller::ptr p = g_context->init_propeller();
  p->go(f, g_context, g_context->init_head(), g_context->init_tail(), pt_);
  spt->set_propagated(true);
}

ArgumentHandle::ArgumentHandle() : offset(0), num(0) {}

char* ArgumentHandle::insert_buf(size_t s) {
  char* b = new char[s];
  bufs.push_back(b);
  return b;
}

ArgumentHandle::~ArgumentHandle() {
  for (long i = 0; i < bufs.size(); i++) delete bufs[i];
}

long retval(sp::SpPoint* pt) {
  return pt->snip()->get_ret_val();
}

}
