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

// This provides a thin CFG layer on top of PatchAPI's

#ifndef _SPCFG_H_
#define _SPCFG_H_

#include "parseAPI/h/CFG.h"
#include "patchAPI/h/PatchCFG.h"
#include "common/common.h"

namespace sp {
	class SpObject;


	// Function, the unit of propagation

	class AGENT_EXPORT SpFunction : public ph::PatchFunction {
	public:
	SpFunction(pe::Function *f,
						 ph::PatchObject* o)
		: ph::PatchFunction(f, o),
			propagated_(false) {}

	SpFunction(const ph::PatchFunction* parFunc,
						 ph::PatchObject* child)
		: ph::PatchFunction(parFunc, child),
			propagated_(false) {}

		virtual ~SpFunction() {}

		// For convenience
		SpObject* GetObject() const;

		// Has instrumentation been propagated to callees of this function?
		bool propagated() const { return propagated_; }
		void SetPropagated(bool b) { propagated_ = b; }

	protected:
		bool propagated_;
	};


	// Block, the unit of instrumentation

	class AGENT_EXPORT SpBlock : public ph::PatchBlock {
	public:
	SpBlock(const ph::PatchBlock *parblk,
					ph::PatchObject *child) 
		: ph::PatchBlock(parblk, child),
			instrumented_(false),
			orig_call_addr_(0) {}

	SpBlock(pe::Block *block,
					ph::PatchObject *obj) 
		: ph::PatchBlock(block, obj),
			instrumented_(false),
			orig_call_addr_(0) {}

		virtual ~SpBlock() {}

		// Has this block been instrumented?
		bool instrumented() const { return instrumented_; }
		void SetInstrumented(bool b) { instrumented_ = b; }

		// Springboard stuffs
		bool IsSpring() const { return is_spring_; }
		void SetIsSpring(bool b) { is_spring_ = b; }

		// Save the whole block
		bool save();

		// For convenience
		SpObject* GetObject() const;
		size_t call_size() const { return (size_t)(end() - last()); }

		// Get original code
		in::Instruction::Ptr orig_call_insn() const { return orig_call_insn_;}

	protected:
		bool instrumented_;
		bool is_spring_;

		dt::Address          orig_call_addr_;
		std::string          orig_code_; // Including last insn
		in::Instruction::Ptr orig_call_insn_;
	};


	// Edge, only for completeness ...

	class AGENT_EXPORT SpEdge : public ph::PatchEdge {
	public:
	SpEdge(pe::Edge *internalEdge,
				 ph::PatchBlock *source,
				 ph::PatchBlock *target) 
		: ph::PatchEdge(internalEdge, source, target) {}

	SpEdge(const ph::PatchEdge *parent,
				 ph::PatchBlock *child_src,
				 ph::PatchBlock *child_trg)
		: ph::PatchEdge(parent, child_src, child_trg) {}

    virtual ~SpEdge() {}
	};
}

#endif /* _SPCFG_H_ */
