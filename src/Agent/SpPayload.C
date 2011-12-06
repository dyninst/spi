#include "SpPayload.h"
#include "SpContext.h"
#include "SpPoint.h"
#include "SpUtils.h"
#include "SpIPC.h"

using ph::PatchFunction;
using ph::Point;
using ph::PatchMgrPtr;
using ph::PatchMgr;
using ph::Scope;
using ph::PatchBlock;
using ph::PatchEdge;
using ph::PatchObject;
using namespace sp;
extern sp::SpContext* g_context;


/* Payload functions wrappers, which will be used only in multi-process mode. */
static bool pre_before(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  /* Sender-side */
  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();

  /* Detect initiation of communication */
  if (!ipc_mgr->is_sender(f->name().c_str())) return true;

  /* Get destination name */
  ArgumentHandle h;
  int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
  SpChannel* c = ipc_mgr->get_channel(*fd);

  /* Inject this agent.so to remote process */
  // ipc_mgr->inject(c);

  if (ipc_mgr->start_tracing()) {
    if (ipc_mgr->is_pipe(*fd)) {
      sp_print("c->remote_pid: %d", c->remote_pid);
      ipc_mgr->set_start_tracing(1, c->remote_pid);
    }
  }

  return true;
}

void wrapper_before(SpPoint* pt, sp::PayloadFunc_t before) {
  if (!pre_before(pt)) return;
  before(pt);
}

static bool pre_after(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  /* Sender-side: detect fork for pipe */
  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();

  if (ipc_mgr->is_fork(f->name().c_str())) {
    long pid = sp::retval(pt);
    /* Receiver */
    if (pid == 0) {
      ipc_mgr->set_start_tracing(0, getpid());
    }
  }

  /* Receipt-side */

  /* Detect receipt */

/*
  // Handle fork, thus pipe
  if (f->name().compare("fork") == 0) {
    long pid = sp::retval(pt);
    if (pid == 0) {
      sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
      ipc_mgr->set_work(0, getpid());
      // Maintain a 32KB shared memory per machine
      // The creation of this shared memory is by SpServer
      // 1 byte for each process's can_work
      // We can support ~32000 processes, which is the maximum for linux
    } else if (pid > 0) {
      g_pid = pid;
    }
  }
*/
  return true;
}

void wrapper_after(SpPoint* pt, sp::PayloadFunc_t after) {
  if (!pre_after(pt)) return;
  after(pt);
}

/* Default payload functions */
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

/* Utilities that payload writers can use in their payload functions */
namespace sp {

/* Get callee from a PreCall point */
ph::PatchFunction* callee(ph::Point* pt) {
  return g_context->callee(pt);
}

/* Pop up an argument of a function call */
void* pop_argument(ph::Point* pt, ArgumentHandle* h, size_t size) {
  return static_cast<SpPoint*>(pt)->snip()->pop_argument(h, size);
}

/* Propel instrumentation to next points of the point `pt` */
void propel(ph::Point* pt) {

  /* Skip if we have already propagated from this point */
  SpPoint* spt = static_cast<sp::SpPoint*>(pt);
  if (spt->propagated()) {
    return;
  }

  PatchFunction* f = callee(pt);
  if (!f) return;

  sp::SpPropeller::ptr p = g_context->init_propeller();
  p->go(f, g_context, g_context->init_before(), g_context->init_after(), pt);
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

bool is_ipc(int fd) {
  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  return ipc_mgr->is_ipc(fd);
}

char start_tracing() {
  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  return ipc_mgr->start_tracing();
}

}
