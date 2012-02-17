#ifndef _SPADDRSPACE_H_
#define _SPADDRSPACE_H_

#include "SpAgentCommon.h"

namespace sp {
class SpObject;
class SpAddrSpace : public ph::AddrSpace {
   friend class SpInstrumenter;
  public:

    static SpAddrSpace* create(ph::PatchObject*);

    virtual dt::Address malloc(ph::PatchObject* obj,
                               size_t           size,
                               dt::Address      near);

    virtual bool write(ph::PatchObject* obj,
                       dt::Address      to,
                       dt::Address      from,
                       size_t           size);

    virtual bool free(ph::PatchObject* obj,
                      dt::Address      orig);

    bool set_range_perm(dt::Address addr,
                        size_t      length,
                        int         perm);

    bool restore_range_perm(dt::Address addr,
                            size_t      length);

    void loadLibrary(ph::PatchObject*);

  protected:

    SpAddrSpace();
};

}

#endif /* _SPADDRSPACE_H_ */
