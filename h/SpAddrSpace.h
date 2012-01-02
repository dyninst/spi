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

    void sp_init(ph::PatchObject*);

    char* get_near(ph::Point* pt);

    void loadLibrary(ph::PatchObject*);
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

    // Preallocated buffer stuffs
    typedef struct {
      dt::Address cursor;  // The addr = pre_head - pre_cursor * buf_size_
      dt::Address head;
    } PreAlloc;
    typedef std::map<ph::PatchObject*, PreAlloc> ObjPreAllocMap;
    ObjPreAllocMap obj_prealloc_map_;
    size_t max_buf_num_;   // maximum # of all buffers before object
    size_t buf_size_;      // each buffer's size

    void update_mem_maps();
    void dump_mem_maps();

    SpAddrSpace();
    void pre_alloc_near(SpObject* obj);
};

}

#endif /* _SPADDRSPACE_H_ */
