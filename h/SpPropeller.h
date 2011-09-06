#ifndef SP_PROPEL_H_
#define SP_PROPEL_H_

#include "SpCommon.h"
#include "SpPayload.h"
#include "PatchCFG.h"

namespace sp {
class SpContext;
class SpPropeller {
  public:
    typedef dyn_detail::boost::shared_ptr<SpPropeller> ptr;
    static ptr create();
    SpPropeller();
    /*
    virtual bool go(Dyninst::PatchAPI::PatchFunction*,
                    SpContextPtr,
                    PayloadFunc);*/
    virtual bool go(Points&  pts,
                    SpContext* context,
                    PayloadFunc payload);
};

}

#endif /* SP_PROPEL_H_ */
