#ifndef _SPADDRSPACE_H_
#define _SPADDRSPACE_H_

#include "AddrSpace.h"

namespace sp {

class SpAddrSpace : public Dyninst::PatchAPI::AddrSpace {
  public:
    static SpAddrSpace* create(Dyninst::PatchAPI::PatchObject*);
    virtual Dyninst::Address malloc(Dyninst::PatchAPI::PatchObject* obj,
                                    size_t size,
                                    Dyninst::Address near);
    virtual bool write(Dyninst::PatchAPI::PatchObject* obj,
                       Dyninst::Address to, Dyninst::Address from,
                       size_t size);
    virtual bool free(Dyninst::PatchAPI::PatchObject* obj,
                      Dyninst::Address orig);

    bool set_range_perm(Dyninst::Address a, size_t length, int perm);
    bool restore_range_perm(Dyninst::Address a, size_t length);

    void sp_init(Dyninst::PatchAPI::PatchObject*);
  protected:

    typedef struct {
      Dyninst::Address start;
      Dyninst::Address end;
      Dyninst::Address offset;
      string dev;
      unsigned long inode;
      int perms;
      string path;
    } MemMapping;
    typedef std::map<Dyninst::Address, MemMapping> MemMappings;
    MemMappings mem_maps_;

    void update_mem_maps();
    public: void dump_mem_maps();

    SpAddrSpace();
};

}

#endif /* _SPADDRSPACE_H_ */
