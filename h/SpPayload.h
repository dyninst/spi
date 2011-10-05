#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "SpCommon.h"
#include "PatchCFG.h"

namespace sp {
  class SpContext;
  typedef void (*PayloadFunc_t)(Dyninst::PatchAPI::Point* pt,
                              SpContext* context);
  typedef void* PayloadFunc;

class SpPayload {
  public:
    SpPayload(Dyninst::PatchAPI::Point*, SpContext* context);

    Dyninst::PatchAPI::PatchFunction* callee();
    void default_propell();
  protected:
    Dyninst::PatchAPI::Point* pt_;
    SpContext* context_;
};

}


#endif /* SP_PAYLOAD_H_ */
