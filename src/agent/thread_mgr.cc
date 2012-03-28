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
#include "agent/propeller.h"
#include "agent/thread_mgr.h"

namespace sp {

extern SpContext* g_context;
extern SpParser* g_parser;
extern SpLock g_propel_lock;

typedef int (*ThreadFunc)(void*);

bool
SpThreadMgr::BeforeEntry(SpPoint* pt) {
  assert(g_context);
  assert(g_parser);

  SpFunction* func = NULL;
  bool ret = true;
  
  SP_LOCK(THREADMGR_BEFOREENTRY);
  sp_debug("THREAD MGR BeforeEntry");
  func = pt->callee();
  if (!func) {
    sp_debug("CANNOT FIND CALLEE FOR %lx", pt->block()->last());
    ret = false;
    goto THREADMGR_BEFOREENTRY_EXIT;
  }

  // Get the thread routine when encountering pthread_create()
  // by looking at the second argument (from left to right)
  if (func->name().compare("pthread_create") == 0) {
    ArgumentHandle handle;
    pt->snip()->PopArgument(&handle, sizeof(void*));
    pt->snip()->PopArgument(&handle, sizeof(void*));
    ThreadFunc* thread_func =
        (ThreadFunc*)pt->snip()->PopArgument(&handle,
                                             sizeof(ThreadFunc));

    SpFunction* tfunc =
        g_parser->FindFunction((dt::Address)*thread_func);
    if (!tfunc) {
      sp_debug("CANNOT FIND FUNCTION FOR %lx", (dt::Address)*thread_func);
      ret = false;
      goto THREADMGR_BEFOREENTRY_EXIT;
    }
    sp_debug("GOT CALLBACK - %s at %lx", tfunc->name().c_str(),
             (long)*thread_func);

    // Start propagating instrumentation to thread routine
    sp::SpPropeller::ptr p = g_context->init_propeller();
    assert(p);
    sp_debug("GET PROPELLER");
    p->go(tfunc,
          g_context->init_entry(),
          g_context->init_exit(),
          NULL);
    tfunc->SetPropagated(true);
  }

  // THREADMGR_BEFOREENTRY_EXIT:  
  SP_UNLOCK(THREADMGR_BEFOREENTRY);
  return ret;
}

}
