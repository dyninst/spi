#ifndef SP_PROPEL_H_
#define SP_PROPEL_H_

#include "SpAgentCommon.h"
#include "SpPayload.h"

namespace sp {
class SpContext;
class SpPropeller {
  public:
    typedef dyn_detail::boost::shared_ptr<SpPropeller> ptr;
    static ptr create();
    SpPropeller();

    bool go(ph::PatchFunction* func,
            SpContext* context,
            PayloadFunc before,
            PayloadFunc after,
            ph::Point* pt = NULL);

  protected:
    //Points pts_;

    virtual void next_points(ph::PatchFunction* func,
                             ph::PatchMgrPtr mgr,
                             Points& pts);
};

}

#endif /* SP_PROPEL_H_ */
