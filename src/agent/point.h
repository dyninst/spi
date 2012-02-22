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

	class SpPoint : public ph::Point {
  public:
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchFunction *f)
		: ph::Point(t,m,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
			ret_addr_(0) {
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
			ret_addr_(0) {
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
			ret_addr_(0) {
    }
	SpPoint(ph::Point::Type t,
					ph::PatchMgrPtr m,
					ph::PatchBlock *b,
					Dyninst::Address a,
					Dyninst::InstructionAPI::Instruction::Ptr i,
					ph::PatchFunction *f)
		: ph::Point(t,m,b,a,i,f),
			tail_call_(false),
			callee_(NULL),
			install_method_(SP_NONE),
			channel_(NULL),
			ret_addr_(0) {
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
			ret_addr_(0) {
    }

    virtual ~SpPoint() { }

    void set_tailcall(bool b) { tail_call_ = b; }
    bool tailcall() { return tail_call_; }

    SpFunction* callee() const { return callee_; }
    void set_callee(SpFunction* f) { callee_ = f; }

    SpSnippet::ptr snip() const { return spsnip_;}
    void set_snip(SpSnippet::ptr s) { spsnip_ = s;}

    InstallMethod install_method() const { return install_method_; }
    void set_install_method(InstallMethod i) { install_method_ = i; }

    void set_channel(sp::SpChannel* c) { channel_ = c; }
    SpChannel* channel() const { return channel_; }

		void set_ret_addr(dt::Address a) { ret_addr_ = a; }
		dt::Address ret_addr() const { return ret_addr_; }

		// For convenience ...
		SpBlock* get_block() const;
		SpObject* get_object() const;

  protected:
    bool tail_call_;
    SpFunction* callee_;
    InstallMethod install_method_;
    SpSnippet::ptr spsnip_;
    SpChannel* channel_;
		dt::Address ret_addr_;
	};

}

#endif /* _SPPOINT_H_ */
