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


// Agent is to manage Agent's configuration parameters

#ifndef SP_AGENT_H_
#define SP_AGENT_H_

#include "common/common.h"

#include "agent/parser.h"
#include "agent/event.h"
#include "agent/payload.h"
#include "agent/propeller.h"

namespace sp {

  class SpAgent : public SHARED_THIS(SpAgent) {
    friend class SpContext;

 public:
    typedef SHARED_PTR(SpAgent) ptr;

    AGENT_EXPORT static ptr Create();
    virtual ~SpAgent();

    // Setters
    AGENT_EXPORT void SetParser(SpParser::ptr);
    AGENT_EXPORT void SetInitEvent(SpEvent::ptr);
    AGENT_EXPORT void SetFiniEvent(SpEvent::ptr);
    AGENT_EXPORT void SetInitEntry(string);
    AGENT_EXPORT void SetInitExit(string);
    AGENT_EXPORT void SetInitPropeller(SpPropeller::ptr);
    AGENT_EXPORT void SetLibrariesToInstrument(const StringSet& libs);

    // Note: We can only bypass instrumenting direct function calls that
    // are specified by this function. For indirect function calls, we
    // still install instrumentation on them (relocate call insn, or call
    // block. However, for indirect function calls, we can skip propagate
    // instrumentation to their callees.
    AGENT_EXPORT void SetFuncsNotToInstrument(const StringSet& funcs);

    AGENT_EXPORT void EnableParseOnly(const bool yes_or_no);
    AGENT_EXPORT void EnableDirectcallOnly(const bool yes_or_no);
    AGENT_EXPORT void EnableTrapOnly(const bool yes_or_no);
    AGENT_EXPORT void EnableIpc(const bool yes_or_no);
    AGENT_EXPORT void EnableHandleDlopen(const bool yes_or_no);
    AGENT_EXPORT void EnableMultithread(const bool yes_or_no);

    // Getters
    AGENT_EXPORT SpParser::ptr parser() const {
      return parser_;
    }
    AGENT_EXPORT SpEvent::ptr init_event() const {
      return init_event_;
    }
    AGENT_EXPORT SpEvent::ptr fini_event() const {
      return fini_event_;
    }
    AGENT_EXPORT std::string init_entry() const {
      return init_entry_;
    }
    AGENT_EXPORT std::string init_exit() const {
      return init_exit_;
    }
    AGENT_EXPORT SpPropeller::ptr init_propeller() const {
      return init_propeller_;
    }
    AGENT_EXPORT const StringSet& libraries_to_instrument() const {
      return libs_to_inst_;
    }
    AGENT_EXPORT const StringSet& funcs_not_to_instrument() const {
      return funcs_not_to_inst_;
    }

    AGENT_EXPORT bool IsParseOnlyEnabled() const {
      return parse_only_;
    }
    AGENT_EXPORT bool IsDirectcallOnlyEnabled() const {
      return directcall_only_;
    }
    AGENT_EXPORT bool IsTrapOnlyEnabled() const {
      return trap_only_;
    }
    AGENT_EXPORT bool IsIpcEnabled() const {
      return allow_ipc_;
    }
    AGENT_EXPORT bool IsMultithreadEnabled() const {
      return allow_multithread_;
    }
    AGENT_EXPORT bool IsHandleDlopenEnabled() const {
      return handle_dlopen_;
    }

    AGENT_EXPORT SpContext* context() const {
      return context_;
    }


    AGENT_EXPORT void Go();

  protected:
    SpEvent::ptr      init_event_;
    SpEvent::ptr      fini_event_;
    SpParser::ptr     parser_;
    SpPropeller::ptr  init_propeller_;
    SpContext*        context_;

    string init_entry_;
    string init_exit_;

    bool parse_only_;
    bool directcall_only_;
    bool allow_ipc_;
    bool trap_only_;
    bool allow_multithread_;
    bool handle_dlopen_;

    StringSet  libs_to_inst_;
    StringSet  funcs_not_to_inst_;

    SpAgent();
  };

}

#endif /* SP_AGENT_H_ */
