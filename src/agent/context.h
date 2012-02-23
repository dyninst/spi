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

#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

#include "common/common.h"
#include "agent/payload.h"
#include "agent/propeller.h"
#include "agent/parser.h"
#include "agent/event.h"
#include "agent/snippet.h"
#include "agent/ipc/ipc_mgr.h"

namespace sp {
typedef std::set<Dyninst::PatchAPI::PatchFunction*> FuncSet;

class SpContext {
  public:
    static SpContext* create(SpPropeller::ptr p,
                             string,
                             string,
                             SpParser::ptr);
    ~SpContext();

    string init_entry_name() const { return init_entry_name_; }
    string init_exit_name() const { return init_exit_name_; }

    PayloadFunc init_entry() const { return init_entry_; }
    PayloadFunc init_exit() const { return init_exit_; }

    PayloadFunc wrapper_entry() const {return wrapper_entry_;}
    PayloadFunc wrapper_exit() const {return wrapper_exit_;}

    SpIpcMgr* ipc_mgr() const { return ipc_mgr_; }
    SpPropeller::ptr   init_propeller() const { return init_propeller_; }

    void get_callstack(FuncSet* func_set);

		// Controlling Instrumentation
    void set_directcall_only(bool b) { directcall_only_ = b; }
    bool directcall_only() const { return directcall_only_; }

    bool allow_ipc() const { return allow_ipc_; }
    void set_allow_ipc(bool b);

  protected:

    PayloadFunc init_entry_;
    PayloadFunc init_exit_;

    string init_entry_name_;
    string init_exit_name_;

    PayloadFunc wrapper_entry_;
    PayloadFunc wrapper_exit_;

    SpIpcMgr* ipc_mgr_;
    SpParser::ptr parser_;
    SpPropeller::ptr init_propeller_;

    bool allow_ipc_;
    bool directcall_only_;

    SpContext(SpPropeller::ptr, SpParser::ptr);
};


}

#endif /* SP_CONTEXT_H_ */
