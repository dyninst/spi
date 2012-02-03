#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

#include "SpAgentCommon.h"
#include "SpPayload.h"
#include "SpPropeller.h"
#include "SpParser.h"
#include "SpEvent.h"
#include "SpSnippet.h"
#include "SpIpcMgr.h"

namespace sp {

class SpContext {
  public:
    static SpContext* create(SpPropeller::ptr p,
                             string,
                             string,
                             SpParser::ptr);
    ~SpContext();

    void parse();

    string           init_entry_name() const { return init_entry_name_; }
    string           init_exit_name() const { return init_exit_name_; }

    PayloadFunc      init_entry() const { return init_entry_; }
    PayloadFunc      init_exit() const { return init_exit_; }

    PayloadFunc      wrapper_entry() const {return wrapper_entry_;}
    PayloadFunc      wrapper_exit() const {return wrapper_exit_;}

    SpIpcMgr*          ipc_mgr() const { return ipc_mgr_; }
    SpParser::ptr      parser() const { return parser_; }

    SpPropeller::ptr   init_propeller() const { return init_propeller_; }
    ph::PatchFunction* callee(ph::Point* pt);

    void get_callstack(FuncSet* func_set);

		// XXX: to remove, by associating info w/ SpBlock 
    typedef std::set<ph::PatchBlock*> SpringSet;
    bool in_spring_set(ph::PatchBlock* b) {
			return (spring_set_.find(b) != spring_set_.end());
		}
    void add_spring(ph::PatchBlock* b) { spring_set_.insert(b); }

		// Controlling Instrumentation
    void set_directcall_only(bool b) { directcall_only_ = b; }
    bool directcall_only() const { return directcall_only_; }

    bool allow_ipc() const { return allow_ipc_; }
    void set_allow_ipc(bool b);

		bool trap_only() const { return trap_only_; }
    void set_trap_only(bool b) { trap_only_ = b; }

		// TODO: should make it elegent by providing CFGMaker
		bool is_blk_instrumented(ph::PatchBlock* b) {
			return (inst_blks_.find(b) != inst_blks_.end());
		}

		void add_blk_instrumented(ph::PatchBlock* b) {
			inst_blks_.insert(b);
		}

		bool is_func_propagated(ph::PatchFunction* f) {
			return (inst_funcs_.find(f) != inst_funcs_.end());
		}

		void add_func_propagated(ph::PatchFunction* f) {
			inst_funcs_.insert(f);
		}

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
		bool trap_only_;
    bool directcall_only_;

    SpringSet spring_set_;
		BlkSet inst_blks_;
		FuncSet inst_funcs_;

    SpContext(SpPropeller::ptr, SpParser::ptr);
};


}

#endif /* SP_CONTEXT_H_ */
