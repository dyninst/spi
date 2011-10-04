#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
  class SpContext;
  typedef void (*PayloadFunc_t)(Dyninst::PatchAPI::Point*,
                              SpContext* context);
  typedef void* PayloadFunc;

}

#endif /* SP_PAYLOAD_H_ */
