/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "agent/context.h"
#include "agent/patchapi/cfg.h"
#include "agent/patchapi/point.h"
#include "agent/payload.h"
#include "common/utils.h"


// The two wrappers are used only in IPC mode
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


// Default payload functions
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


// Utilities that payload writers can use in their payload functions
namespace sp {
extern SpContext* g_context;
extern SpParser::ptr g_parser;
extern SpLock* g_propel_lock;

// Get callee from a PreCall point
SpFunction*
callee(SpPoint* pt) {


  bool parse_indirect = true;

  // If we just want to instrument direct call, then we skip parsing
  // indirect calls
  if (g_context->IsDirectcallOnlyEnabled()) parse_indirect = false;

  SpFunction* f = g_parser->callee(pt, parse_indirect);

  return f;
}

// Pop up an argument of a function call
void*
pop_argument(SpPoint* pt, ArgumentHandle* h, size_t size) {

  void* arg = PT_CAST(pt)->snip()->pop_argument(h, size);

  return arg;
}

// Propel instrumentation to next points of the point `pt`
void
propel(SpPoint* pt) {

  int result = Lock(g_propel_lock);
  
  if (result == SP_DEAD_LOCK) {
    sp_print("DEAD LOCK - skip instrumentation for insn %lx",
             pt->block()->last());
    Unlock(g_propel_lock);
    return;
  }

  if (!pt->getCallee()) {
    Unlock(g_propel_lock);
    return;
  }
  
  SpFunction* f = callee(pt);
  if (!f) {
    sp_debug("NOT VALID FUNC - stop propagation");
    Unlock(g_propel_lock);
    return;
  }

  // Skip if we have already propagated from this point
  if (f->propagated()) {
    Unlock(g_propel_lock);
    return;
  }

  sp::SpPropeller::ptr p = g_context->init_propeller();
  assert(p);

  p->go(f,
        g_context->init_entry(),
        g_context->init_exit(),
        pt);
  f->SetPropagated(true);

  Unlock(g_propel_lock);
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
  long ret = pt->snip()->get_ret_val();
  return ret;
}

// Implicitly call start_tracing()
bool
is_ipc_write(SpPoint* pt) {
  SpChannel* c = pt->channel();
  bool ret = (c && c->rw == SP_WRITE);
  return ret;
}

// Implicitly call start_tracing()
bool
is_ipc_read(SpPoint* pt) {
  SpChannel* c = pt->channel();

  if (callee(pt)->name().compare("accept") != 0)
    return (c && c->rw == SP_READ && start_tracing(c->fd));

  bool ret = (c && c->rw == SP_READ);

  return ret;
}

char
start_tracing(int fd) {

  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  char ret = ipc_mgr->start_tracing(fd);

  return ret;
}

}
