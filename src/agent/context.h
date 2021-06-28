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

// This provides the "context" for self propelled instrumentation.
// Essentially, it contains a copy of variables set in SpAgent.
// SpAgent instance would be gone immediately in the agent shared library.
// However, this SpContext instance will remain during the life time of
// self-propelled instrumentation.

#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

#include "common/common.h"
#include "agent/payload.h"
#include "agent/propeller.h"
#include "agent/parser.h"
#include "agent/event.h"
#include "agent/snippet.h"
#include "agent/ipc/ipc_mgr.h"

#include "frame.h"
#include "walker.h"
#include <stack>

namespace sp {

class SpContext {
  friend class SpAgent;
  public:
    static SpContext* Create();
    ~SpContext();

    // Getters
    string init_entry_name() const {
      return init_entry_name_;
    }
    string init_exit_name() const {
      return init_exit_name_;
    }
    PayloadFunc init_entry() const {
      return init_entry_;
    }
    PayloadFunc init_exit() const {
      return init_exit_;
    }
    PayloadFunc wrapper_entry() const {
      return wrapper_entry_;
    }
    PayloadFunc wrapper_exit() const {
      return wrapper_exit_;
    }
    SpIpcMgr* ipc_mgr() const {
      return ipc_mgr_;
    }
    SpPropeller::ptr init_propeller() const {
      return init_propeller_;
    }

    bool IsDirectcallOnlyEnabled() const {
      return directcall_only_;
    }
    bool IsIpcEnabled() const {
      return allow_ipc_;
    }
    bool IsHandleDlopenEnabled() const {
      return handle_dlopen_;
    }
    bool IsMultithreadEnabled() const {
      return allow_multithread_;
    }

    std::string getAgentName() const {
      return parser_->agent_name();
    }

    void GetCallStack(FuncSet* func_set);

    SpPoint* FindCallSitePointFromRetAddr(dt::Address ret);
    char* FindExitInstAddrFromCallSitePoint(SpPoint*);
   
    //Table to store the non-instrumented functions and their return address
    typedef std::map<SpPoint*,char*> PointRetAddrMap;
    typedef std::map<dt::Address, SpPoint*> RetAddrCallPointMap;
    PointRetAddrMap pt_ra_map_;
    RetAddrCallPointMap ra_csp_map_;

    dt::Address GetReturnAddress();

    void PushPointCallInfo(PointCallInfo*);
    PointCallInfo* PopPointCallInfo();

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
    bool handle_dlopen_;
    bool allow_multithread_;
    bool directcall_only_;

    //Stackwalking variables
    sk::Walker *walker_;
    std::vector<sk::Frame> stackwalk_;

    SpContext();

    // Setters, we only allow SpAgent to use these setters
    
    void EnableDirectcallOnly(bool b) {
      directcall_only_ = b;
    }

    void EnableIpc(bool b) {
      allow_ipc_ = b;
    }

    void EnableHandleDlopen(bool b) {
      handle_dlopen_ = b;
    }
    
    void EnableMultithread(bool b) {
      allow_multithread_ = b;
    }

    void SetInitEntryName(std::string name) {
      init_entry_name_ = name;
    }
    
    void SetInitExitName(std::string name) {
      init_exit_name_ = name;
    }

    void SetParser(SpParser::ptr p) {
      parser_ = p;
    }

    void SetInitPropeller(SpPropeller::ptr p) {
      init_propeller_ = p;
    }
    
    void SetInitEntry(PayloadFunc f) {
      assert(f);
      init_entry_ = f;
    }
    
    void SetInitExit(PayloadFunc f) {
      assert(f);
      init_exit_ = f;
    }
    
    void SetWrapperEntry(PayloadFunc f) {
      wrapper_entry_ = f;
    }
    
    void SetWrapperExit(PayloadFunc f) {
      wrapper_exit_ = f;
    }
    
    void SetIpcMgr(SpIpcMgr* mgr) {
      ipc_mgr_ = mgr;
    }
    
    void init_propeller(SpPropeller::ptr p) {
      init_propeller_ = p;
    }
};


}

#endif /* SP_CONTEXT_H_ */
