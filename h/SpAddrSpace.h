#ifndef _SPADDRSPACE_H_
#define _SPADDRSPACE_H_

#include "SpAgentCommon.h"

namespace sp {

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

    void sp_init(ph::PatchObject*);
  protected:

    typedef struct {
      dt::Address start;
      dt::Address end;
      dt::Address offset;
      string dev;
      unsigned long inode;
      int perms;
      string path;
    } MemMapping;

    typedef std::map<dt::Address, MemMapping> MemMappings;
    MemMappings mem_maps_;

    void update_mem_maps();
    void dump_mem_maps();

    SpAddrSpace();
};

}

#endif /* _SPADDRSPACE_H_ */
