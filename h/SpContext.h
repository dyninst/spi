#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

//#include <ext/hash_map>

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
    static SpContext* create(SpPropeller::ptr,
                             string,
                             string,
                             SpParser::ptr);
    ~SpContext();

    void parse();
    void restore();

    SpIpcMgr*        ipc_mgr() const { return ipc_mgr_; }
    PayloadFunc      init_before() const { return init_before_; }
    PayloadFunc      init_after() const { return init_after_; }
    PayloadFunc      wrapper_before() const {return wrapper_before_;}
    PayloadFunc      wrapper_after() const {return wrapper_after_;}
    SpParser::ptr    parser() const { return parser_; }
    ph::PatchMgrPtr  mgr() const { return mgr_; }
    SpPropeller::ptr init_propeller() const { return init_propeller_; }

    ph::PatchFunction* callee(ph::Point* pt);
    ph::PatchFunction* get_first_inst_func();

    void set_old_act(struct sigaction old_act) { old_act_ = old_act; }

    bool is_well_known_lib(string);

    typedef std::set<ph::PatchBlock*> SpringSet;
    bool in_spring_set(ph::PatchBlock* b) { return (spring_set_.find(b) != spring_set_.end()); }
    void add_spring(ph::PatchBlock* b) { spring_set_.insert(b); }

    void set_directcall_only(bool b) { directcall_only_ = b; }
    bool directcall_only() const { return directcall_only_; }

    bool allow_ipc() const { return allow_ipc_; }
    void set_allow_ipc(bool b);

  protected:
    SpPropeller::ptr init_propeller_;
    PayloadFunc init_before_;
    PayloadFunc init_after_;

    SpParser::ptr parser_;
    ph::PatchMgrPtr mgr_;
    std::vector<std::string> well_known_libs_;
    bool directcall_only_;

    // Things to be restored
    struct sigaction old_act_;
    SpringSet spring_set_;

    PayloadFunc wrapper_before_;
    PayloadFunc wrapper_after_;

    bool allow_ipc_;
    SpIpcMgr* ipc_mgr_;

    SpContext(SpPropeller::ptr,
              SpParser::ptr);
    void init_well_known_libs();
};


}

#endif /* SP_CONTEXT_H_ */
