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

#include "common/utils.h"

namespace sp {
  extern SpContext* g_context;
  extern SpParser::ptr g_parser;

  // Default Event -- dumb event, does nothing
  SpEvent::SpEvent() {
  }

  void
  SpEvent::RegisterEvent() {
  }

  // AsyncEvent
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

  // SyncEvent
  SyncEvent::SyncEvent(std::string func_name)
    : SpEvent(), func_name_(func_name) {
  }


  void
  SyncEvent::RegisterEvent() {
    bool fail_preload = false;
    if (!g_parser->injected()) {

      sp_debug("PRELOAD - preload agent.so, and instrument main()");

      ph::PatchFunction* f = g_parser->FindFunction("main");
      if (f) {
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
      } else {
        sp_debug("FAIL PRELOAD - try injection ...");
        fail_preload = true;
      }
    } // LD_PRELOAD mode

    if (g_parser->injected() || fail_preload) {

      // Instrument all functions in the call stack.
      FuncSet call_stack;
      g_context->GetCallStack(&call_stack);
      sp_debug("CALLSTACK - %lu calls in the call stack",
               (unsigned long)call_stack.size());
      for (FuncSet::iterator i = call_stack.begin(); 
           i != call_stack.end(); i++) {
        ph::PatchFunction* f = *i;
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
        // We instrument all functions along the call stack, until main
        if (f->name().compare("main") == 0) {
          break;
        }
      } // Call stack
    } // Injection mode
  }


  // Pre-instrument curtain functions
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
      sp::SpFunction* f = g_parser->FindFunction(*i);
      if (f) funcs_.insert(f);
    }

    for (FuncSet::iterator i = funcs_.begin(); 
           i != funcs_.end(); i++) {
        ph::PatchFunction* f = *i;
        sp_print("PRE-INST FUNC - %s", f->name().c_str());
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
    }
  }


} // namespace
