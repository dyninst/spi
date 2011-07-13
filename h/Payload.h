#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "Common.h"

namespace sp {
class Payload {
  public:
    typedef dyn_detail::boost::shared_ptr<Payload> ptr;
    static ptr create();
    virtual bool operator()();
};
}

#endif /* SP_PAYLOAD_H_ */
