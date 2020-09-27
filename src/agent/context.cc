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

#include "injector/injector.h"

#include "agent/context.h"
#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/point.h"
#include "agent/propeller.h"
#include "common/utils.h"


#include <execinfo.h>

namespace sp {

  extern SpParser::ptr g_parser;
  extern SpLock* g_propel_lock;
  extern SpAddrSpace* g_as;

  SpContext::SpContext() :
      init_entry_(NULL),
      init_exit_(NULL),
      wrapper_entry_(NULL),
      wrapper_exit_(NULL),
      ipc_mgr_(NULL),
      parser_(SpParser::ptr()),
      init_propeller_(SpPropeller::ptr()),
      allow_ipc_(false),
      allow_multithread_(false),
      directcall_only_(false),
      walker_(sk::Walker::newWalker()) {

  }

  SpContext*
  SpContext::Create() {
    SpContext* ret = new SpContext();
    assert(ret);
    return ret;
  }

  // Get the first instrumentable function.
  // Here, an instrumentable function should fulfill all of the following
  // requirements:
  //  1. it should be resovled by the parser.
  //  2. it should not be from some well known system libraries
  // TODO (wenbin): use gstack to avoid the stackwalker bug on 32-bit x86
  void
  SpContext::GetCallStack(FuncSet* call_stack) {
    /*
    void* buffer[100];
    int num = backtrace(buffer, 100);
    sp_debug("%d traces", num);
    char** syms = backtrace_symbols(buffer, num);
    for (int i = 0; i < num; i++) {
      sp_debug("%lx - %s", *(unsigned long*)buffer[i], syms[i]);
    }
    return;
    */
    
    long pc, sp, bp;
    parser_->GetFrame(&pc, &sp, &bp);
    SpFunction* func = parser_->FindFunction(pc);
    if (func) sp_print("%s", func->name().c_str());

    sp_debug("GET FRAME - pc: %lx, sp: %lx, bp: %lx", pc, sp, bp);
    sk::Frame* f = sk::Frame::newFrame(pc, sp, bp, walker_);
    sp_debug("constructed frame");
    assert(walker_);

    //Get the total number of calls in stack using StackWalkerAPI
    walker_->walkStackFromFrame(stackwalk_, *f);
    sp_debug("WALKED STACK - %ld function calls found",
             (long)stackwalk_.size());

    for (unsigned i=0; i<stackwalk_.size(); i++) {
      string s;
      stackwalk_[i].getName(s);

    
      FuncSet found_funcs;
      g_parser->FindFunction(s, &found_funcs);
      for (FuncSet::iterator fi = found_funcs.begin();
           fi != found_funcs.end(); fi++) {
        if (*fi) call_stack->insert(*fi);
      }

      /*
      // Step 1: if the function can be resolved
      SpFunction* func = parser_->FindFunction(s.c_str());
      if (!func) {
        sp_debug("SKIPPED - Function %s cannot be resolved", s.c_str());
        continue;
      }

      // Step 2: add this function
      sp_debug("FOUND - Function %s is in the call stack", s.c_str());
      call_stack->insert(func);
      */
    }
  }

  //Get the return address of the first recv like function which is on the stack
  dt::Address SpContext::GetReturnAddress()
  {
	for(unsigned i=0;i <stackwalk_.size() ;i++) {
		string func;
		stackwalk_[i].getName(func);
		if(IsRecvLikeFunction(func.c_str())) {
			sk::location_t location =stackwalk_[i+1].getRALocation();
			sp_debug("Function %s Return Address on the stack  = %lx",func.c_str(),location.val.addr);
			return (dt::Address) stackwalk_[i+1].getRA();
		}
	}
	return (dt::Address)0; 
  }

  //The function name says it all
  SpPoint* SpContext::FindCallSitePointFromRetAddr(dt::Address ret_addr)
  {
	if(ra_csp_map_.find(ret_addr) == ra_csp_map_.end())
		return NULL;
	else
		return ra_csp_map_[ret_addr];
  }
  
  char* SpContext::FindExitInstAddrFromCallSitePoint(SpPoint* pt)
  {
	if(pt_ra_map_.find(pt) == pt_ra_map_.end())	
		return NULL;
	else
		return pt_ra_map_[pt];
  }

  SpContext::~SpContext() {
    delete ipc_mgr_;
    delete g_propel_lock;
    delete g_as;
  }
}
