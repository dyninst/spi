#ifndef SP_PROPEL_H_
#define SP_PROPEL_H_

#include "Common.h"
#include "Payload.h"

namespace sp {
class Propeller {
  public:
    typedef dyn_detail::boost::shared_ptr<Propeller> ptr;
    static ptr create();

    typedef enum {
      CALLEE              // Current function's callee
    } PointType;

    virtual bool go(PointType, Payload::ptr);
    void set_context(ContextPtr c);
 protected:
    Propeller();

    ContextPtr context_;
};
}

#endif /* SP_PROPEL_H_ */
