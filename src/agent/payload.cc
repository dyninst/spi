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

#include <atomic>         // std::atomic

using std::string;

// global variable to track if we should
// do instrumentation, the purpose of this is to stop
// instrumentation when we hit exit function
std::atomic<int> IN_INSTRUMENTATION {1};

namespace sp {
extern SpContext* g_context;
extern SpParser::ptr g_parser;

PointHandle::PointHandle(SpPoint* pt, SpFunction* callee, void* user_info, long ret_val) {
  this->pt_ = pt;
  this->callee_ = callee;
  this->user_info_ = user_info;
  this->return_value_ = ret_val;
}

PointHandle::~PointHandle() {}

SpPoint* PointHandle::GetPoint() {
  return pt_;
}

SpFunction* PointHandle::GetCallee() {
  return callee_;
}

void* PointHandle::GetUserInfo() {
  return user_info_;
}

long PointHandle::ReturnValue() {
  return return_value_;
}
}

//////////////////////////////////////////////////////////////////////

/**
 * wrapper function for payload entry
 * This wrapper is responsible for getting the callee of current point
 * and calling the payload entry function. It gets the returned pointer
 * from payload entry and push the callee and pointer to the stack,
 * the callee and pointer are expected to be popped off from stack in
 * the wrapper_exit function
 */
void
wrapper_entry(sp::SpPoint* pt,
              sp::PayloadFuncEntry entry) {
  sp_debug("In wrapper entry function for point %p", pt);
  if (sp::g_context == NULL) {
    sp_perror("Global context is NULL, return");
    return;
  }

  void* user_info = NULL;
  sp::SpFunction* callee = sp::Callee(pt);

  if (callee != NULL) {
    // Handle IPC stuffs
    if (sp::g_context->IsIpcEnabled()) {
      sp::SpIpcMgr::BeforeEntry(pt);
    }

    // Handle multihread stuffs
    if (sp::g_context->IsMultithreadEnabled()) {
      sp::SpThreadMgr::BeforeEntry(pt);
    }

    if (entry) {
      user_info = entry(pt);
    }
  }

  sp::PointCallInfo* pointInfo = new sp::PointCallInfo(pt, callee, user_info);
  sp::g_context->PushPointCallInfo(pointInfo);
  sp_debug("Pushing point call info onto stack: %lx", pointInfo->pt);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * wrapper function for payload exit
 * This wrapper is responsible for retrieving the callee information from the 
 * stack and passing the information into the user-defined payload exit
 * function.
 */
void
wrapper_exit(sp::SpPoint* pt,
             sp::PayloadFuncExit exit) {
  sp_debug("In wrapper exit function for point %p", pt);

  sp::PointCallInfo* pointInfo;
  long ret_val;
  
  // Retrieving point information and call payload exit function
  pointInfo = sp::g_context->PopPointCallInfo();

  // get return value for this point, also for all the tailcalls before the point
  ret_val = ReturnValue(pt);

  // iterate through tailcall points
  while (pt != pointInfo->pt) {
    sp_debug("Got info from previous tail call %p", pointInfo->pt);

    if (!pointInfo->pt->tailcall()) {
      sp_debug("ERROR: this point is not tailcall");
    }

    sp::PointHandle* point_handle =
      new sp::PointHandle(pointInfo->pt, pointInfo->callee, pointInfo->info, ret_val);
    
    // Handle IPC stuffs
    if (sp::g_context->IsIpcEnabled()) {
      sp::SpIpcMgr::BeforeExit(point_handle);
    }

    // Handle dlopen
    if (sp::g_context->IsHandleDlopenEnabled()) {
      sp::SpParser::ParseDlExit(pointInfo->pt);
    }

    if (exit) {
      exit(point_handle);
    }
    
    delete point_handle;
    delete pointInfo;
    pointInfo = sp::g_context->PopPointCallInfo();
  }

  sp::PointHandle* point_handle =
      new sp::PointHandle(pointInfo->pt, pointInfo->callee, pointInfo->info, ret_val);

  // Now, deal with the correct pointInfo
  if (sp::g_context->IsIpcEnabled()) {
    sp::SpIpcMgr::BeforeExit(point_handle);
  }

  // Handle dlopen
  if (sp::g_context->IsHandleDlopenEnabled()) {
    sp::SpParser::ParseDlExit(pt);
  }

  if (exit) {
    exit(point_handle);
  }

  delete point_handle;
  delete pointInfo;
}


//////////////////////////////////////////////////////////////////////

// Default payload functions
void*
default_entry(sp::SpPoint* pt) {
  ph::PatchFunction* f = sp::Callee(pt);
  if (!f) return NULL;

  string callee_name = f->name();
  sp_print("Enter %s", callee_name.c_str());

  sp::Propel(pt);
  return NULL;
}

//////////////////////////////////////////////////////////////////////

void
default_exit(sp::PointHandle*) {
  
/*    ph::PatchFunction* f = sp::Callee(pt);
    if (!f) return;

    string callee_name = f->name();
    sp_print("Leave %s", callee_name.c_str());
 */
}

/**
 * payload entry function for exit function to toggle off
 * instrumentation
 */
void
toggle_off_instrumentation_entry(sp::SpPoint* pt) {
  sp_debug("PROCESS[%d] toggle off instrumentation", getpid());
  IN_INSTRUMENTATION = 0;
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
  // the instrumentation status will be off after the exit function is called
  // we stop instrumentation since the exit handlers call destructors for
  // all the global variables, and we can not rely on them any more
  if (!IN_INSTRUMENTATION) {
    sp_debug("PROCESS[%d] Already hit exit function, skip instrumentation", getpid());
    return;
  } else {
    sp_debug("PROCESS[%d] still in instrumentation", getpid());
  }

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
    sp_debug("Already propagated, goto exit");
    goto PROPEL_EXIT;
  }

  p = g_context->init_propeller();
  if (!p) {
    sp_debug("asserting propeller");
    assert(p);
  }

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
static char StartTracingNolock(int fd);

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

bool
IsIpcRead(SpPoint* pt) {
  bool ret = false;

  SpChannel* c = NULL;
  SP_LOCK(ISIPCREAD);
  c = pt->channel();

  if (c && c->rw == SP_READ && StartTracingNolock(c->fd)) {
    // if (c && c->rw == SP_READ) {
    ret = true;
    if (CalleeNolock(pt)->name().compare("accept") == 0) {
      sp_debug("Accept skip");
      ret = false;
    }
  }

  SP_UNLOCK(ISIPCREAD);

  return ret;
}

//////////////////////////////////////////////////////////////////////

// Allow to run user-provided trace code?
static char
StartTracingNolock(int fd) {

  sp::SpIpcMgr* ipc_mgr = g_context->ipc_mgr();
  char ret = ipc_mgr->CanStartTracing(fd);

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
