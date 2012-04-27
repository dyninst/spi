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

#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"

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
      directcall_only_(false) {

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
    long pc, sp, bp;
    parser_->GetFrame(&pc, &sp, &bp);
    sp_debug("GET FRAME - pc: %lx, sp: %lx, bp: %lx", pc, sp, bp);
    std::vector<sk::Frame> stackwalk;
    sk::Walker *walker = sk::Walker::newWalker();
    sk::Frame* f = sk::Frame::newFrame(pc, sp, bp, walker);
    walker->walkStackFromFrame(stackwalk, *f);
    sp_debug("WALKED STACK - %ld function calls found",
             (long)stackwalk.size());

    /*
    char cmd[255];
    snprintf(cmd, 255, "pstack %d", getpid());
    FILE* fp = popen(cmd, "r");
    char buf[1024];
    while(fgets(buf, 1024, fp) != NULL) {
      sp_print(buf);
    }
    pclose(fp);
    */
    
    for (unsigned i=0; i<stackwalk.size(); i++) {
      string s;
      stackwalk[i].getName(s);

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

  SpContext::~SpContext() {
    delete ipc_mgr_;
    delete g_propel_lock;
    delete g_as;
  }
}
