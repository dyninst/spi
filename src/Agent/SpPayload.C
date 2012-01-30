#include "SpPayload.h"
#include "SpContext.h"
#include "SpPoint.h"
#include "SpUtils.h"

using ph::Point;
using ph::Scope;
using ph::PatchMgr;
using ph::PatchEdge;
using ph::PatchBlock;
using ph::PatchMgrPtr;
using ph::PatchObject;
using ph::PatchFunction;

using sp::SpPoint;
using sp::SpChannel;
using sp::ArgumentHandle;
using sp::SpIpcMgr;

void
wrapper_entry(SpPoint* pt, sp::PayloadFunc_t entry) {
  if (!SpIpcMgr::before_entry(pt)) return;
  entry(pt);
}


void
wrapper_exit(SpPoint* pt, sp::PayloadFunc_t exit) {
  if (!SpIpcMgr::before_exit(pt)) return;
  exit(pt);
}

// Default payload functions
void
default_entry(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::propel(pt);
}

void
default_exit(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Leave %s", callee_name.c_str());
}

// Utilities that payload writers can use in their payload functions
namespace sp {
  extern SpContext* g_context;

  // Get callee from a PreCall point
  PatchFunction*
  callee(ph::Point* pt) {
    return g_context->callee(pt);
  }

  // Pop up an argument of a function call
  void*
  pop_argument(ph::Point* pt, ArgumentHandle* h, size_t size) {
    return static_cast<SpPoint*>(pt)->snip()->pop_argument(h, size);
  }

  // Propel instrumentation to next points of the point `pt`
  void
  propel(ph::Point* pt) {

    /* Skip if we have already propagated from this point */
    SpPoint* spt = static_cast<sp::SpPoint*>(pt);
    if (spt->propagated()) {
      return;
    }

    PatchFunction* f = callee(pt);
    if (!f) return;

    sp::SpPropeller::ptr p = g_context->init_propeller();
    p->go(f, g_context, g_context->init_entry(), g_context->init_exit(), pt);
    spt->set_propagated(true);
  }

  ArgumentHandle::ArgumentHandle() : offset(0), num(0) {}

  char*
  ArgumentHandle::insert_buf(size_t s) {
    char* b = new char[s];
    bufs.push_back(b);
    return b;
  }

  ArgumentHandle::~ArgumentHandle() {
    for (unsigned i = 0; i < bufs.size(); i++) delete bufs[i];
  }

  long
  retval(sp::SpPoint* pt) {
    return pt->snip()->get_ret_val();
  }

	// Implicitly call start_tracing()
  bool
  is_ipc_write(SpPoint* pt) {
    SpChannel* c = pt->channel();
    return (c && c->rw == SP_WRITE);
  }

	// Implicitly call start_tracing()
  bool
  is_ipc_read(SpPoint* pt) {
    SpChannel* c = pt->channel();

    if (callee(pt)->name().compare("accept") != 0)
			return (c && c->rw == SP_READ && start_tracing(c->fd));

		return (c && c->rw == SP_READ);
  }

  char
  start_tracing(int fd) {
    sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
    return ipc_mgr->start_tracing(fd);
  }

}
