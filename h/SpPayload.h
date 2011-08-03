#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
  typedef bool (*PayloadFunc)(Dyninst::PatchAPI::PatchFunction* cur_func,
                              SpContext* context);

  bool default_payload(Dyninst::PatchAPI::PatchFunction* cur_func,
                       SpContext* context);

class SpPayload {
  public:
    typedef dyn_detail::boost::shared_ptr<SpPayload> ptr;
    SpPayload();
    static ptr create();
};
}

#endif /* SP_PAYLOAD_H_ */
