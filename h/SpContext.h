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
