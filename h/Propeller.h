#ifndef SP_PROPEL_H_
#define SP_PROPEL_H_

#include "Common.h"

namespace sp {
class Propeller {
  public:
    typedef dyn_detail::boost::shared_ptr<Propeller> ptr;
    static ptr create();
    virtual bool operator()();
};
}

#endif /* SP_PROPEL_H_ */
