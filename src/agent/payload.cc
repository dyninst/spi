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
#include "agent/thread_mgr.h"
#include "common/utils.h"

namespace sp {
extern SpContext* g_context;
extern SpParser::ptr g_parser;
}

//////////////////////////////////////////////////////////////////////

// The two wrappers are used only in IPC mode
void
wrapper_entry(sp::SpPoint* pt,
              sp::PayloadFunc_t entry) {
  assert(sp::g_context);

  // Handle IPC stuffs
  if (sp::g_context->IsIpcEnabled()) {
    if (!sp::SpIpcMgr::BeforeEntry(pt)) {
      return;
    }
  }

  // Handle multihread stuffs
  if (sp::g_context->IsMultithreadEnabled()) {
    if (!sp::SpThreadMgr::BeforeEntry(pt)) {
      return;
    }
  }
  entry(pt);
}


//////////////////////////////////////////////////////////////////////

void
wrapper_exit(sp::SpPoint* pt,
             sp::PayloadFunc_t exit) {
  if (!sp::SpIpcMgr::BeforeExit(pt)) return;
  exit(pt);
}


//////////////////////////////////////////////////////////////////////

// Default payload functions
void
default_entry(sp::SpPoint* pt) {
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::Propel(pt);
}

//////////////////////////////////////////////////////////////////////

void
default_exit(sp::SpPoint* pt) {
  /*
    ph::PatchFunction* f = sp::Callee(pt);
    if (!f) return;

    string callee_name = f->name();
    sp_print("Leave %s", callee_name.c_str());
  */
}

// Utilities that payload writers can use in their payload functions

namespace sp {

// Global lock to synchronize instrumentation
// TODO (wenbin): could we have finer grained lock?
SpLock g_propel_lock;

//////////////////////////////////////////////////////////////////////

// Argument handle
ArgumentHandle::ArgumentHandle()
    : offset(0), num(0) {
}

//////////////////////////////////////////////////////////////////////

char*
ArgumentHandle::insert_buf(size_t s) {
  char* b = new char[s];
  bufs.push_back(b);
  return b;
}

//////////////////////////////////////////////////////////////////////

ArgumentHandle::~ArgumentHandle() {
  for (unsigned i = 0; i < bufs.size(); i++) delete bufs[i];
}

//////////////////////////////////////////////////////////////////////

// Get callee from a PreCall point
static SpFunction*
CalleeNolock(SpPoint* pt) {
  if (!pt) return NULL;

  bool parse_indirect = true;

  // If we just want to instrument direct call, then we skip parsing
  // indirect calls
  if (g_context->IsDirectcallOnlyEnabled()) parse_indirect = false;

  SpFunction* f = g_parser->callee(pt, parse_indirect);

  return f;
}

//////////////////////////////////////////////////////////////////////

SpFunction*
Callee(SpPoint* pt) {
  SpFunction* ret = NULL;
  SP_LOCK(CALLEE);
  ret = CalleeNolock(pt);
  SP_UNLOCK(CALLEE);
  return ret;
}

//////////////////////////////////////////////////////////////////////

// Propel instrumentation to next points of the point `pt`
void
Propel(SpPoint* pt) {

  sp::SpPropeller::ptr p = sp::SpPropeller::ptr();
  SpFunction* f = NULL;

  SP_LOCK(PROPEL);
  f = CalleeNolock(pt);
  if (!f) {
    sp_debug("NOT VALID FUNC - stop propagation");
    goto PROPEL_EXIT;
  }

  // Skip if we have already propagated from this point
  if (f->propagated()) {
    goto PROPEL_EXIT;
  }

  p = g_context->init_propeller();
  assert(p);

  p->go(f,
        g_context->init_entry(),
        g_context->init_exit(),
        pt);
  f->SetPropagated(true);
  SP_UNLOCK(PROPEL);
}

//////////////////////////////////////////////////////////////////////

// Get arguments
void*
PopArgument(SpPoint* pt,
            ArgumentHandle* h,
            size_t size) {
  void* arg = NULL;
  SP_LOCK(POPARG);
  arg = PT_CAST(pt)->snip()->PopArgument(h, size);
  SP_UNLOCK(POPARG);
  return arg;
}

//////////////////////////////////////////////////////////////////////

// Get return value
long
ReturnValue(sp::SpPoint* pt) {
  long ret = -1;
  SP_LOCK(RETVAL);
  ret = pt->snip()->GetRetVal();
  SP_UNLOCK(RETVAL);
  return ret;
}

//////////////////////////////////////////////////////////////////////

bool
IsIpcWrite(SpPoint* pt) {
  SpChannel* c = NULL;
  bool ret = false;
  SP_LOCK(ISIPCWRITE);
  c = pt->channel();
  ret = (c && c->rw == SP_WRITE);
  SP_UNLOCK(ISIPCWRITE);
  return ret;
}

//////////////////////////////////////////////////////////////////////

// Implicitly call start_tracing()
static char StartTracingNolock(int fd);

bool
IsIpcRead(SpPoint* pt) {
  bool ret = false;

  SpChannel* c = NULL;
  SP_LOCK(ISIPCREAD);
  c = pt->channel();

  if (c && c->rw == SP_READ) {
    ret = true;
    if (CalleeNolock(pt)->name().compare("accept") == 0) ret = false;
  }

  SP_UNLOCK(ISIPCREAD);

  return ret;
}

//////////////////////////////////////////////////////////////////////

// Allow to run user-provided trace code?
static char
StartTracingNolock(int fd) {

  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  char ret = ipc_mgr->start_tracing(fd);

  return ret;
}

//////////////////////////////////////////////////////////////////////

char
StartTracing(SpPoint* pt,
             int fd) {

  char ret = 0;
  SP_LOCK(STARTTRACING);
  ret = StartTracingNolock(fd);
  SP_UNLOCK(STARTTRACING);
  return ret;
}

//////////////////////////////////////////////////////////////////////

bool
IsInstrumentable(SpPoint* pt) {
  assert(g_parser);
  bool ret = true;
  SpFunction* func = NULL;
  SP_LOCK(ISINSTRUMENTABLE);
  func = CalleeNolock(pt);
  if (!func) {
    ret = false;
  } else if (!g_parser->CanInstrumentFunc(func->name())) {
    ret = false;
  }
  SP_UNLOCK(ISINSTRUMENTABLE);
  return ret;
}

}
