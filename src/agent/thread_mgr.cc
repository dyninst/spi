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

typedef int (*CloneCallback)(void*);

bool
SpThreadMgr::BeforeEntry(SpPoint* pt) {
  assert(g_context);
  assert(g_parser);
  
  SpFunction* func = NULL;
  
  SP_LOCK(THREADMGR_BEFOREENTRY);
  sp_debug("THREAD MGR BeforeEntry");
  func = pt->callee();
  if (!func) {
    goto THREADMGR_BEFOREENTRY_EXIT;
    return false;
  }

  if (func->name().compare("__clone") == 0) {
    ArgumentHandle handle;
    CloneCallback* callback =
        (CloneCallback*)pt->snip()->PopArgument(&handle,
                                                sizeof(CloneCallback));
    SpFunction* callback_func =
        g_parser->FindFunction((dt::Address)*callback);
    if (!callback_func) {
      goto THREADMGR_BEFOREENTRY_EXIT;
      return false;
    }
    sp_print("GOT CALLBACK - %s at %lx", callback_func->name().c_str(),
             (long)*callback);

    sp::SpPropeller::ptr p =   p = g_context->init_propeller();
    p->go(callback_func,
          g_context->init_entry(),
          g_context->init_exit(),
          NULL);
    callback_func->SetPropagated(true);
  }
  SP_UNLOCK(THREADMGR_BEFOREENTRY);
  return true;
}

}
