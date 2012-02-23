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

    // Getters
		AGENT_EXPORT SpParser::ptr parser() const {
      return parser_;
    }

    // Setters
    AGENT_EXPORT void SetParser(SpParser::ptr);
    AGENT_EXPORT void SetInitEvent(SpEvent::ptr);
    AGENT_EXPORT void SetFiniEvent(SpEvent::ptr);
    AGENT_EXPORT void SetInitEntry(string);
    AGENT_EXPORT void SetInitExit(string);
    AGENT_EXPORT void SetInitPropeller(SpPropeller::ptr);
    AGENT_EXPORT void SetLibrariesToInstrument(const StringSet& libs);
        
    AGENT_EXPORT void EnableParseOnly(bool yes_or_no);
    AGENT_EXPORT void EnableDirectcallOnly(bool yes_or_no);
		AGENT_EXPORT void EnableTrapOnly(bool yes_or_no);
    AGENT_EXPORT void EnableIpc(bool yes_or_no);

    AGENT_EXPORT void Go();

  protected:
    SpEvent::ptr      init_event_;
    SpEvent::ptr      fini_event_;
    SpParser::ptr     parser_;
    SpPropeller::ptr  init_propeller_;

    string init_entry_;
    string init_exit_;

    bool parse_only_;
    bool directcall_only_;
    bool allow_ipc_;
		bool trap_only_;

    StringSet  libs_to_inst_;
    
    SpAgent();
	};

}

#endif /* SP_AGENT_H_ */
