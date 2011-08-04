#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
    typedef bool (*PayloadFunc)(Dyninst::PatchAPI::PatchFunction* cur_func,
                                SpContext* context);
  // typedef void* PayloadFunc;

}

extern "C" {
bool default_payload(Dyninst::PatchAPI::PatchFunction* cur_func,
                     sp::SpContext* context);
void simple_payload();
}
#endif /* SP_PAYLOAD_H_ */
