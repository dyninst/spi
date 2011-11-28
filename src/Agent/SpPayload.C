#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpPoint.h"
#include "SpUtils.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchObject;

//-----------------------------------------
// Payload functions wrappers
//-----------------------------------------
static bool pre_before(Point* pt) {
  // Detect IPC primitives
  // TODO

  return true;
}

void wrapper_before(Point* pt, sp::PayloadFunc_t before) {
  if (!pre_before(pt)) return;
  before(pt);
}

static bool pre_after(Point* pt) {
  return true;
}

void wrapper_after(Point* pt, sp::PayloadFunc_t after) {
  if (!pre_after(pt)) return;
  after(pt);
}

//-----------------------------------------
// Default payload functions
//-----------------------------------------
void default_before(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::propel(pt);
}

void default_after(Point* pt) {
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
  p->go(f, g_context, g_context->init_before(), g_context->init_after(), pt_);
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
