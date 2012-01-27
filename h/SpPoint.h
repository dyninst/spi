#ifndef _SPPOINT_H_
#define _SPPOINT_H_

#include "SpAgentCommon.h"
#include "SpSnippet.h"

namespace sp {

	class SpChannel;

	typedef enum {
		SP_NONE,
		SP_TRAP,      
		SP_RELOC_INSN,
		SP_RELOC_BLK, 
		SP_SPRINGBOARD
	} InstallMethod;

	class SpPoint : public ph::Point {
  public:
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchFunction *f)
		: ph::Point(t,m,f), propagated_(false), instrumented_(false),
			tail_call_(false), callee_(NULL), install_method_(SP_NONE),
			channel_(NULL), ret_addr_(0) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchFunction *f,
					ph::PatchBlock *b)
		: ph::Point(t,m,f,b), propagated_(false), instrumented_(false),
			tail_call_(false), callee_(NULL), install_method_(SP_NONE),
			channel_(NULL), ret_addr_(0) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchBlock *b,
					ph::PatchFunction *f)
		: ph::Point(t,m,b,f), propagated_(false), instrumented_(false),
			tail_call_(false), callee_(NULL), install_method_(SP_NONE),
			channel_(NULL), ret_addr_(0) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchBlock *b,
					Dyninst::Address a,
					Dyninst::InstructionAPI::Instruction::Ptr i,
					ph::PatchFunction *f)
		: ph::Point(t,m,b,a,i,f), propagated_(false), instrumented_(false),
			tail_call_(false), callee_(NULL), install_method_(SP_NONE),
			channel_(NULL), ret_addr_(0) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchEdge *e,
					ph::PatchFunction *f)
		: ph::Point(t,m,e,f), propagated_(false), instrumented_(false),
			tail_call_(false), callee_(NULL), install_method_(SP_NONE),
			channel_(NULL), ret_addr_(0) {
    }

    virtual ~SpPoint() { }
    void set_propagated(bool b) { propagated_ = b; }
    bool propagated() { return propagated_; }

    void set_instrumented(bool b) { instrumented_ = b; }
    bool instrumented() { return instrumented_; }

    void set_tailcall(bool b) { tail_call_ = b; }
    bool tailcall() { return tail_call_; }

    ph::PatchFunction* callee() const { return callee_; }
    void set_callee(ph::PatchFunction* f) { callee_ = f; }
    SpSnippet::ptr snip() const { return spsnip_;}
    void set_snip(SpSnippet::ptr s) { spsnip_ = s;}

    InstallMethod install_method() const { return install_method_; }
    void set_install_method(InstallMethod i) { install_method_ = i; }

    void set_channel(sp::SpChannel* c) { channel_ = c; }
    SpChannel* channel() const { return channel_; }

		// For future undo
		in::Instruction::Ptr orig_call_insn() const { return orig_call_insn_; }
		void set_orig_call_insn(in::Instruction::Ptr i) { orig_call_insn_ = i; }

		void set_ret_addr(dt::Address a) { ret_addr_ = a; }
		dt::Address ret_addr() const { return ret_addr_; }
  protected:
    bool propagated_;
    bool instrumented_;
    bool tail_call_;
    ph::PatchFunction* callee_;
    InstallMethod install_method_;
    SpSnippet::ptr spsnip_;
    sp::SpChannel* channel_;

		// Things for future undo (uninstrumentation)

		// - For TrapWorker/RelocCallInsnWorker, it stores the original 
    //   call instruction
		in::Instruction::Ptr orig_call_insn_;

		// - For RelocCallBlockWorker/SpringboardWorker, it stores the
    //   original call block
		dt::Address call_blk_addr_;
		string orig_call_blk_;
	
		// Springboard used for this point
		dt::Address spring_addr_;   // The start address of spring board
		string orig_spring_;        // The original springboard block

		dt::Address ret_addr_;
	};

}

#endif /* _SPPOINT_H_ */
