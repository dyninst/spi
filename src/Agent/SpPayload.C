#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpPoint.h"
#include "SpUtils.h"
#include "SpIPC.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchObject;
using namespace sp;
extern sp::SpContext* g_context;

//-----------------------------------------
// Payload functions wrappers
//-----------------------------------------
int g_pid = 0;
static bool pre_before(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return false;

  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  if (ipc_mgr->can_work()) {
    if (f->name().compare("read") == 0 ||
	f->name().compare("write") == 0) {
      ArgumentHandle h;
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      // sp_print("fd: %d", *fd);
      if (ipc_mgr->is_pipe(*fd)) {
	sp_print("It's a pipe!");
	// Let child process's payload function work
	ipc_mgr->set_work(1, g_pid);
      } else if (ipc_mgr->is_tcp(*fd)) {
      } else if (ipc_mgr->is_udp(*fd)) {
      }
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

  return true;
}

void wrapper_after(SpPoint* pt, sp::PayloadFunc_t after) {
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

bool can_work() {
  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  return ipc_mgr->can_work();
}
}
