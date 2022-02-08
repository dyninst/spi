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

#include <iterator>

#include "agent/event.h"
#include "agent/context.h"
#include "agent/parser.h"
#include "agent/patchapi/object.h"

#include "PatchMgr.h"
#include "PatchObject.h"

#include "common/utils.h"

namespace sp {
  extern SpContext* g_context;
  extern SpParser::ptr g_parser;

// ------------------------------------------------------------------- 
// Default Event -- dumb event, does nothing
// -------------------------------------------------------------------
  SpEvent::SpEvent() {
  }

  void
  SpEvent::RegisterEvent() {
  }

// ------------------------------------------------------------------- 
// Combo event -- a set of events
// -------------------------------------------------------------------
  CombEvent::CombEvent(EventSet& events) {
    std::copy(events.begin(), events.end(),
              std::inserter(events_, events_.begin()));
  }

  void
  CombEvent::RegisterEvent() {
    for (EventSet::iterator ei = events_.begin();
         ei != events_.end(); ei++) {
      (*ei)->RegisterEvent();
    }
  }


// ------------------------------------------------------------------- 
// AsyncEvent
// -------------------------------------------------------------------
  typedef void (*event_handler_t)(int, siginfo_t*, void*);

  void
  async_event_handler(int signum, siginfo_t* info, void* context) {
    assert(0 && "TODO");
  }

  AsyncEvent::AsyncEvent(int signum, int sec)
    : after_secs_(sec), signum_(signum) {
    handler_ = (void*)async_event_handler;
  }

  void
  AsyncEvent::RegisterEvent() {
    struct sigaction act;
    act.sa_sigaction = (event_handler_t)handler_;
    act.sa_flags = SA_SIGINFO;
    sigaction(signum_, &act, NULL);
    if (signum_ == SIGALRM) alarm(after_secs_);
  }

// ------------------------------------------------------------------- 
// SyncEvent
// -------------------------------------------------------------------
  SyncEvent::SyncEvent()
    : SpEvent(){
  }


  void
  SyncEvent::RegisterEvent() {
    bool fail_preload = false;
    /* Determine if the function is injected or preloaded by looking
       at the call stack size. If the call stack size is zero then
       the agent is preloaded. If the call stack size is greater 
       than zero, then the agent is injected */
    FuncSet call_stack;
    g_context->GetCallStack(&call_stack);
    sp_debug("CALLSTACK - %lu calls in the call stack",
              (unsigned long)call_stack.size());

    if (call_stack.size() <= 0)  {
    sp_debug("PRELOAD - preload agent.so, and instrument main()");
    sp_debug(GetExeName().c_str());
      
      FuncSet found_funcs;
      SpFunction* f = NULL;
      g_parser->FindFunctionByMangledName("main", &found_funcs);
      for (FuncSet::iterator i = found_funcs.begin(); i != found_funcs.end(); i++) {
        if (strcmp(FUNC_CAST(*i)->GetObject()->name().c_str(), sp::GetExeObjName().c_str()) == 0)
          f = FUNC_CAST(*i);
      }
      //f = g_parser->FindFunction("main");
      if (f) {
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());

      } else {
        sp_debug("FAIL PRELOAD - try injection ...");
        fail_preload = true;
      }
    } // LD_PRELOAD mode


    if ( call_stack.size()>0  || fail_preload) {
      // Instrument all functions in the call stack.
      // Instrument all return points in the calls currently in the stack
      for (FuncSet::iterator i = call_stack.begin(); 
           i != call_stack.end(); i++) {
        SpFunction* f = *i;
        sp_debug("Call stck function %s",f->name().c_str());
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
        // We instrument all functions along the call stack, until main
        if (f->name().compare("main") == 0) {
          break;
        }
        if (IsRecvLikeFunction(f->name())) {
          sp_debug("Recv like function on the stack");
          //Modify the PC to start at a new location 
          g_context->init_propeller()->ModifyPC(f,g_context->init_exit());	
        }
      } // Iterate through all the functions int the Call stack
    } // Injection mode
 
  }

// ------------------------------------------------------------------- 
// Pre-instrument curtain functions
// -------------------------------------------------------------------

  FuncEvent::FuncEvent(StringSet& funcs) {
    std::copy(funcs.begin(), funcs.end(),
              std::inserter(func_names_, func_names_.begin()));
  }

  void
  FuncEvent::RegisterEvent() {
    SetSegfaultSignal();

    for (StringSet::iterator i = func_names_.begin();
         i != func_names_.end(); i++) {
      // sp_print("%s", (*i).c_str());
      FuncSet found_funcs;
      g_parser->FindFunction(*i, &found_funcs);
      for (FuncSet::iterator fi = found_funcs.begin();
           fi != found_funcs.end(); fi++) {
        if (*fi) funcs_.insert(*fi);
      }
    }

    for (FuncSet::iterator i = funcs_.begin();
           i != funcs_.end(); i++) {
        SpFunction* f = *i;
        sp_debug("PRE-INST FUNC - %s", f->name().c_str());
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
    }
  }


// ------------------------------------------------------------------- 
// Pre-instrument certain calls
// -------------------------------------------------------------------

  CallEvent::CallEvent(StringSet& funcs) {
    std::copy(funcs.begin(), funcs.end(),
              std::inserter(func_names_, func_names_.begin()));
  }

  void
  CallEvent::RegisterEvent() {
    SetSegfaultSignal();

    assert(g_parser);
    ph::PatchMgrPtr mgr = g_parser->mgr();
    assert(mgr);
    ph::AddrSpace* as = mgr->as();
    assert(as);
    
    FuncSet func_set;
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
      SpObject* obj = OBJ_CAST(ci->second);
      assert(obj);

      if (strcmp(sp_filename(obj->name().c_str()), "libagent.so") == 0) {
        sp_debug("SKIP - lib %s", sp_filename(obj->name().c_str()));
        continue;
      }
      
      if (!g_parser->CanInstrumentLib(sp_filename(obj->name().c_str()))) {
        sp_debug("SKIP - lib %s", sp_filename(obj->name().c_str()));
        continue;
      }
      // sp_print("HANDLING - lib %s", sp_filename(obj->name().c_str()));

      pe::CodeObject* co = obj->co();
      assert(co);

      const pe::CodeObject::funclist& all = co->funcs();
      for (pe::CodeObject::funclist::iterator fit = all.begin();
           fit != all.end(); fit++) {
        SpFunction* found = FUNC_CAST(obj->getFunc(*fit));
        if (!found) continue;

        g_context->init_propeller()->go(found,
                                        g_context->init_entry(),
                                        g_context->init_exit(),
                                        NULL,
                                        &func_names_);
      } // For each ParseAPI::Function
    } // For each PatchObject
  }

} // namespace
