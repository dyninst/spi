#ifndef SP_CONTEXT_H_
#define SP_CONTEXT_H_

#include "SpCommon.h"
#include "SpPayload.h"
#include "SpPropeller.h"
#include "SpParser.h"
#include "PatchMgr.h"
#include "SpEvent.h"

namespace sp {

class SpContext {
  public:
  static SpContextPtr create(SpPropeller::ptr,
                             SpPayload::ptr,
                             SpParser::ptr);

    SpContext(SpPropeller::ptr,
              SpPayload::ptr,
              SpParser::ptr);

    SpPayload::ptr init_payload() { return init_payload_; }
    // SpPropeller::ptr init_propeller() { return init_propeller_; }
    bool propel(int, SpPayload::ptr);

    void parse();

    Dyninst::PatchAPI::PatchFunction* get_first_inst_func();
    Dyninst::PatchAPI::PatchMgrPtr mgr() { return mgr_; }

  protected:
    SpPropeller::ptr propeller_;
    SpPayload::ptr init_payload_;
    SpParser::ptr parser_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    std::vector<string> well_known_libs_;

    void init_well_known_libs();
    bool is_well_known_lib(string);
};


}

#endif /* SP_CONTEXT_H_ */
