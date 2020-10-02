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

#ifndef _SPPOINT_H_
#define _SPPOINT_H_

#include "common/common.h"
#include "agent/snippet.h"

namespace sp {

	class SpChannel;
	class SpObject;

	typedef enum {
		SP_NONE,
		SP_TRAP,      
		SP_RELOC_INSN,
		SP_RELOC_BLK, 
		SP_SPRINGBOARD
	} InstallMethod;

	class AGENT_EXPORT SpPoint : public ph::Point {
  public:
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchFunction *f)
		: ph::Point(t,m,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
      ret_addr_(0),
      caller_point_(NULL) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchFunction *f,
					ph::PatchBlock *b)
		: ph::Point(t,m,f,b),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
      ret_addr_(0),
      caller_point_(NULL) {
      
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchBlock *b,
					ph::PatchFunction *f)
		: ph::Point(t,m,b,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
      ret_addr_(0),
      caller_point_(NULL) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchBlock *b,
					Dyninst::Address a,
					Dyninst::InstructionAPI::Instruction i,
					ph::PatchFunction *f)
		: ph::Point(t,m,b,a,i,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
      ret_addr_(0),
      caller_point_(NULL) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchEdge *e,
					ph::PatchFunction *f)
		: ph::Point(t,m,e,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
      ret_addr_(0),
      caller_point_(NULL) {
    }

    virtual ~SpPoint() { }

    void SetIsTailcall(bool b) { tail_call_ = b; }
    bool tailcall() { return tail_call_; }

    SpFunction* callee() const { return callee_; }
    void SetCallee(SpFunction* f) { callee_ = f; }

    void SetSnip(SpSnippet::ptr s) { spsnip_ = s;}
    SpSnippet::ptr snip() const { return spsnip_;}

    void SetInstallMethod(InstallMethod i) { install_method_ = i; }
    InstallMethod install_method() const { return install_method_; }

    void SetChannel(sp::SpChannel* c) { channel_ = c; }
    SpChannel* channel() const { return channel_; }

		void SetRetAddr(dt::Address a) { ret_addr_ = a; }
		dt::Address ret_addr() const { return ret_addr_; }

    void SetCallerPt(SpPoint* pt) { caller_point_ = pt; }
    SpPoint* caller_pt() { return caller_point_; }
    
		// For convenience ...
		SpBlock* GetBlock() const;
		SpObject* GetObject() const;

  protected:
    bool tail_call_;
    SpFunction* callee_;
    InstallMethod install_method_;
    SpSnippet::ptr spsnip_;
    SpChannel* channel_;
		dt::Address ret_addr_;
    SpPoint* caller_point_;
	};

}

#endif /* _SPPOINT_H_ */
