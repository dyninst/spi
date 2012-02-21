#ifndef _SPOBJECT_H_
#define _SPOBJECT_H_

#include "SpCommon.h"

#include "PatchObject.h"

// PatchAPI stuffs.
namespace sp {

	// Structures for memory allocator
	typedef std::list<unsigned> FreeList;

	typedef enum {
		SMALL_BUF,
		MID_BUF,
		BIG_BUF
	} BufType;

	typedef std::map<dt::Address, BufType> BufTypeMap;

	typedef struct {
		dt::Address base;   // The base address for all these buffers
		size_t buf_size;    // Each buffer's size
		FreeList list;      // A list of offsets for free buffers
	} FreeBufs;

  class SpObject : public ph::PatchObject {
		friend class SpParser;
		friend class SpAddrSpace;
  public:

    SpObject(pe::CodeObject* o,
						 dt::Address a,
						 ph::CFGMaker* cm,
             ph::PatchCallback* cb,
						 dt::Address la,
						 sb::Symtab* symtab)
			: ph::PatchObject(o, a, cm, cb),
			load_addr_(la),
			symtab_(symtab) {}

    dt::Address load_addr() const { return load_addr_; }

		// Get this object's name
		std::string name();

		// Get this object's associated symtab
		sb::Symtab* symtab() const { return symtab_; }

  protected:
    dt::Address load_addr_;
		std::string name_;
		sb::Symtab* symtab_;

		FreeBufs small_freebufs_;  // Small buffers
		FreeBufs mid_freebufs_;    // Midium buffers
		FreeBufs big_freebufs_;    // Big buffers
		BufTypeMap alloc_bufs_;    // To facilitate future deallocation

		void init_memory_alloc(dt::Address base,
													 size_t size);
		dt::Address get_freebuf(size_t size);
		bool put_freebuf(dt::Address buf);
};

}

#endif /* _SPOBJECT_H_ */
