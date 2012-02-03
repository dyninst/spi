#include "SpCFG.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpPayload.h"
#include "SpContext.h"

// ------------------------------------------------------------------- 
// The two wrappers are used only in IPC mode
// -------------------------------------------------------------------
void
wrapper_entry(sp::SpPoint* pt, sp::PayloadFunc_t entry) {
  if (!sp::SpIpcMgr::before_entry(pt)) return;
  entry(pt);
}


void
wrapper_exit(sp::SpPoint* pt, sp::PayloadFunc_t exit) {
  if (!sp::SpIpcMgr::before_exit(pt)) return;
  exit(pt);
}

// ------------------------------------------------------------------- 
// Default payload functions
// -------------------------------------------------------------------
void
default_entry(sp::SpPoint* pt) {
	ph::PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::propel(pt);
}

void
default_exit(sp::SpPoint* pt) {
	ph::PatchFunction* f = sp::callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Leave %s", callee_name.c_str());
}

// ------------------------------------------------------------------- 
// Utilities that payload writers can use in their payload functions
// -------------------------------------------------------------------
namespace sp {
  extern SpContext* g_context;
	extern SpParser::ptr g_parser;

  // Get callee from a PreCall point
	SpFunction*
  callee(SpPoint* pt) {
		bool parse_indirect = true;

		// If we just want to instrument direct call, then we skip parsing
		// indirect calls
		if (g_context->directcall_only()) parse_indirect = false;

    return g_parser->callee(pt, parse_indirect);
  }

  // Pop up an argument of a function call
  void*
  pop_argument(SpPoint* pt, ArgumentHandle* h, size_t size) {
    return static_cast<SpPoint*>(pt)->snip()->pop_argument(h, size);
  }

  // Propel instrumentation to next points of the point `pt`
  void
  propel(SpPoint* pt) {

    SpFunction* f = callee(pt);
    if (!f) {
			sp_debug("NOT VALID FUNC - stop propagation");
			return;
		}

    // Skip if we have already propagated from this point
		if (f->propagated()) return;

    sp::SpPropeller::ptr p = g_context->init_propeller();
		assert(p);

    p->go(f,
					g_context->init_entry(),
					g_context->init_exit(),
					pt);
		f->set_propagated(true);
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
