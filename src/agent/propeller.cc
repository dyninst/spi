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
#include "agent/patchapi/point.h"
#include "agent/propeller.h"
#include "agent/snippet.h"
#include "common/utils.h"

#include "patchAPI/h/Command.h"
#include "patchAPI/h/PatchMgr.h"

namespace sp {

  extern SpContext* g_context;
  extern SpParser::ptr g_parser;

  SpPropeller::SpPropeller() {
  }

  SpPropeller::ptr
  SpPropeller::Create() {
    return ptr(new SpPropeller);
  }

  // Instrument all "interesting points" inside `func`.
  // "Interesting points" are provided by SpPropeller::next_points.
  // By default, next_points provides all call instructions of `func`.
  // However, users can inherit SpPropeller and implement their own
  // next_points().
  bool
  SpPropeller::go(ph::PatchFunction* func,
                  PayloadFunc entry,
                  PayloadFunc exit,
                  ph::Point* pt) {
    assert(func);

    sp_debug("START PROPELLING - propel to callees of function %s",
             func->name().c_str());

    // 1. Find points according to type
    Points pts;
    ph::PatchMgrPtr mgr = g_parser->mgr();
    assert(mgr);
    ph::PatchFunction* cur_func = NULL;
    if (pt) {
      cur_func = func;
    } else {
      cur_func = g_parser->FindFunction(func->name(),
                                        func->addr());
    }

    if (!cur_func) return false;
    
    next_points(cur_func, mgr, pts);

    // 2. Start instrumentation
    ph::Patcher patcher(mgr);
    for (unsigned i = 0; i < pts.size(); i++) {
      SpPoint* pt = PT_CAST(pts[i]);
      assert(pt);
      SpBlock* blk = pt->GetBlock();
      assert(blk);

      if (blk->isShared()) {
        if (blk->instrumented()) {
          continue;
        }
      }

      // It's possible that callee will be NULL, which is an indirect call.
      // In this case, we'll parse it later during runtime.

      SpFunction* callee = g_parser->callee(pt);

      if (callee) {
        if (!g_parser->CanInstrumentFunc(callee->name())) {
          sp_debug("SKIP NOT-INST FUNC - %s", callee->name().c_str());
          continue;
        }
        sp_debug("POINT - instrumenting direct call at %lx to "
                 "function %s (%lx) for point %lx",
                 blk->last(), callee->name().c_str(),
                 (dt::Address)callee, (dt::Address)pt);
      } else {
        sp_debug("POINT - instrumenting indirect call at %lx for point %lx",
                 blk->last(), (dt::Address)pt);
      }

      sp_debug("PAYLOAD ENTRY - %lx", (long)entry);
      SpSnippet::ptr sp_snip = SpSnippet::create(callee,
                                                 pt,
                                                 entry,
                                                 exit);
      ph::Snippet<SpSnippet::ptr>::Ptr snip =
        ph::Snippet<SpSnippet::ptr>::create(sp_snip);
      assert(sp_snip && snip);
      pt->SetSnip(sp_snip);
      patcher.add(ph::PushBackCommand::create(pt, snip));
    }
    bool ret = patcher.commit();

    if (ret) {
      sp_debug("FINISH PROPELLING - callees of function %s are"
               " instrumented", func->name().c_str());
    } else {
      sp_debug("FINISH PROPELLING - instrumentation failed for"
               " callees of %s", func->name().c_str());
    }
    return ret;
  }

  // Find all PreCall points
  void
  SpPropeller::next_points(ph::PatchFunction* cur_func,
                           ph::PatchMgrPtr mgr,
                           Points& pts) {
    ph::Scope scope(cur_func);
    mgr->findPoints(scope, ph::Point::PreCall, back_inserter(pts));
  }

}
