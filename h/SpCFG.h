#ifndef _SPCFG_H_
#define _SPCFG_H_

#include "CFG.h"
#include "PatchCFG.h"
#include "SpAgentCommon.h"

namespace sp {
	class SpObject;

	// ------------------------------------------------------------------- 
	// Function, the unit of propagation
	// -------------------------------------------------------------------

	class SpFunction : public ph::PatchFunction {
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
		SpObject* get_object() const;

		// Has instrumentation been propagated to callees of this function?
		bool propagated() const { return propagated_; }
		void set_propagated(bool b) { propagated_ = b; }

	protected:
		bool propagated_;
	};

	// ------------------------------------------------------------------- 
	// Block, the unit of instrumentation
	// -------------------------------------------------------------------

	class SpBlock : public ph::PatchBlock {
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
		void set_instrumented(bool b) { instrumented_ = b; }

		// Springboard stuffs
		bool is_spring() const { return is_spring_; }
		void set_is_spring(bool b) { is_spring_ = b; }

		// Save the whole block
		bool save();

		// For convenience
		SpObject* get_object() const;
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

	// ------------------------------------------------------------------- 
	// Edge
	// -------------------------------------------------------------------

	class SpEdge : public ph::PatchEdge {
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
