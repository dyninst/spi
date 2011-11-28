#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

//#include <ext/hash_map>

#include "SpCommon.h"
#include "SpPayload.h"
#include "SpPropeller.h"
#include "SpParser.h"
#include "PatchMgr.h"
#include "SpEvent.h"
#include "SpSnippet.h"

namespace sp {

class SpContext {
  public:
  static SpContext* create(SpPropeller::ptr,
                           string,
                           string,
                           SpParser::ptr);


    PayloadFunc init_before() { return init_before_; }
    PayloadFunc init_after() { return init_after_; }

    SpPropeller::ptr init_propeller() { return init_propeller_; }
    bool propel(int, PayloadFunc);

    void parse();
    void restore();

    Dyninst::PatchAPI::PatchFunction* callee(Dyninst::PatchAPI::Point* pt);
    SpParser::ptr parser() { return parser_; }
    Dyninst::PatchAPI::PatchFunction* get_first_inst_func();
    Dyninst::PatchAPI::PatchMgrPtr mgr() { return mgr_; }
    void set_old_act(struct sigaction old_act) { old_act_ = old_act; }

    // EIP -> snippet
    typedef std::map<Dyninst::Address, Dyninst::PatchAPI::InstancePtr> InstMap;
    InstMap& inst_map() { return inst_map_; }

    typedef std::set<Dyninst::PatchAPI::PatchBlock*> SpringSet;
    bool in_spring_set(Dyninst::PatchAPI::PatchBlock* b) { return (spring_set_.find(b) != spring_set_.end()); }
    void add_spring(Dyninst::PatchAPI::PatchBlock* b) { spring_set_.insert(b); }

    bool is_well_known_lib(string);
    void set_directcall_only(bool b) { directcall_only_ = b; }
    bool directcall_only() { return directcall_only_; }

    PayloadFunc wrapper_before() const {return wrapper_before_;}
    PayloadFunc wrapper_after() const {return wrapper_after_;}

  protected:
    SpPropeller::ptr init_propeller_;
    PayloadFunc init_before_;
    PayloadFunc init_after_;

    SpParser::ptr parser_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    std::vector<string> well_known_libs_;
    bool directcall_only_;

    // Things to be restored
    struct sigaction old_act_;
    InstMap inst_map_;
    SpringSet spring_set_;

    PayloadFunc wrapper_before_;
    PayloadFunc wrapper_after_;

    SpContext(SpPropeller::ptr,
              SpParser::ptr);
    void init_well_known_libs();
};


}

#endif /* SP_CONTEXT_H_ */
