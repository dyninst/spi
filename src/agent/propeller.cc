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
#include "agent/inst_workers/inst_worker_delegate.h" 
#include "agent/inst_workers/trap_worker_impl.h"

#include "Command.h"
#include "PatchMgr.h"

#include <signal.h>
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
  SpPropeller::go(SpFunction* func,
                  PayloadFunc entry,
                  PayloadFunc exit,
                  SpPoint* point,
                  StringSet* inst_calls) {
    if (func == NULL) {
      sp_debug("FATAL: func is NULL");
      return false;
    }

    sp_debug("START PROPELLING - propel to callees of function %s",
             func->name().c_str());
   
    if (func->name().find("std::")!=std::string::npos || func->name().find("cxx")!=std::string::npos) {
      sp_debug("TODO: libstdc++ functions: stop propelling");
      return true;
    }

    // Skip propelling into functions that we do not want to instrument
    // We need this check if we are doing initial instrumentation for
    // all functions on the stack trace when agentlib is injected
    if (!g_parser->CanInstrumentFunc(func->name())) {
      sp_debug("SKIP propel into - %s", func->name().c_str());
      return false;
    }

    // 1. Find points according to type
    Points pts;
    ph::PatchMgrPtr mgr = g_parser->mgr();
    assert(mgr);
    ph::PatchFunction* cur_func = func;

    if (!cur_func) return false;

    next_points(cur_func, mgr, pts);

    // 2. Start instrumentation
    ph::Patcher patcher(mgr);
    for (unsigned i = 0; i < pts.size(); i++) {
      SpPoint* p = PT_CAST(pts[i]);
      assert(p);
      SpBlock* blk = p->GetBlock();
      assert(blk);

      if (blk->instrumented()) {
        continue;
      }
      
      // It's possible that callee will be NULL, which is an indirect call.
      // In this case, we'll parse it later during runtime.

      SpFunction* callee = g_parser->callee(p);

      if (callee) {
        if (!g_parser->CanInstrumentFunc(callee->name())) {
          sp_debug("SKIP NOT-INST FUNC - %s", callee->name().c_str());
          continue;
        }

        // Only works for direct function calls
        if (inst_calls &&
            (inst_calls->find(callee->name()) == inst_calls->end())) {
          // sp_print("SKIP NOT-INST CALL - %s", callee->name().c_str());
          sp_debug("SKIP NOT-INST CALL - %s", callee->name().c_str());
          continue;
        }
        sp_debug("POINT - instrumenting direct call at %lx to "
                 "function %s (%lx) for point %lx",
                 blk->last(), callee->name().c_str(),
                 (dt::Address)callee, (dt::Address)p);
      } else {
        if (inst_calls) {
          sp_debug("SKIP INDIRECT CALL - at %lx", blk->last());
          continue;
        }
        sp_debug("POINT - instrumenting indirect call at %lx for point %lx",
                 blk->last(), (dt::Address)p);
      }

      sp_debug("PAYLOAD ENTRY - %lx", (long)entry);
      p->SetCallerPt(point);
      SpSnippet::ptr sp_snip = SpSnippet::create(callee,
                                                 p,
                                                 entry,
                                                 exit);
      /*
      ph::Snippet<SpSnippet::ptr>::Ptr snip =
        ph::Snippet<SpSnippet::ptr>::create(sp_snip);
      assert(sp_snip && snip);
      p->SetSnip(sp_snip);
      patcher.add(ph::PushBackCommand::create(p, snip));
      */
      assert(sp_snip);
      p->SetSnip(sp_snip);
      patcher.add(ph::PushBackCommand::create(p, sp_snip));
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
 

 //For all recv like functions which are on the stack, install a trap at the ret points 
 // and modify the PC to go to the exit point instrumentation.
  bool
  SpPropeller::ModifyPC(SpFunction* func,
                  PayloadFunc exit) {
    assert(func);
    sp_debug("Modify PC for the function %s",func->name().c_str());
	
    Points pts;
    ph::PatchMgrPtr mgr = g_parser->mgr();
    assert(mgr);
    ph::PatchFunction* cur_func = NULL;
    cur_func = g_parser->FindFunction(func->GetMangledName());

    if (!cur_func) return false;
    //1. Find all return points
    next_ret_points(cur_func, mgr, pts);
    sp_debug("No of return points for the function %s  is %lu", func->name().c_str(),pts.size());
    
    //2.. Replace all the return points associated with the function with a trap instruction
    for(unsigned i=0; i<pts.size();i++) {
		//Install a trap at the return points and modify the call 
		SpPoint* p = PT_CAST(pts[i]);
		TrapWorker* trap=new TrapWorker;
	        trap->ReplaceReturnWithTrap(p);
		//The trap handling code will take care of modifying the PC to the 
		//corresponding instruction.Check in trap_worker_impl.cc
    }
    return true;
  }

  // Find all PreCall points
  void
  SpPropeller::next_points(ph::PatchFunction* cur_func,
                           ph::PatchMgrPtr mgr,
                           Points& pts) {
    ph::Scope scope(cur_func);
    mgr->findPoints(scope, ph::Point::PreCall, back_inserter(pts));
  }
  
  //Find all function exit points
  void
  SpPropeller::next_ret_points(ph::PatchFunction* cur_func,
	                   ph::PatchMgrPtr mgr,
			   Points& pts) {
   ph::Scope scope(cur_func);
   mgr->findPoints(scope, ph::Point::FuncExit,back_inserter(pts));
  }
		 	   
}
