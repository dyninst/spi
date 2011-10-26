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

    bool go(Dyninst::PatchAPI::PatchFunction* func,
            SpContext* context,
            PayloadFunc head,
            PayloadFunc tail,
            Dyninst::PatchAPI::Point* pt = NULL);

  protected:
    //Points pts_;

    virtual void next_points(Dyninst::PatchAPI::PatchFunction* func,
                             Dyninst::PatchAPI::PatchMgrPtr mgr,
                             Points& pts);
};

}

#endif /* SP_PROPEL_H_ */
