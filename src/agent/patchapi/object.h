/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _SPOBJECT_H_
#define _SPOBJECT_H_

#include "common/common.h"

#include "PatchObject.h"
#include "agent/patchapi/addr_space.h"


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

  class AGENT_EXPORT SpObject : public ph::PatchObject {
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
		std::string name() const {
      assert(symtab_);
      return symtab_->name();
    }

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
    typedef std::map<dt::Address, size_t> BufSizeMap;  // For the mmap-ed
    BufSizeMap buf_size_map_;
        
		void InitMemoryAlloc(dt::Address base,
												 size_t size);
		dt::Address AllocateBuffer(size_t size);
		bool FreeBuffer(dt::Address buf);
};

}

#endif /* _SPOBJECT_H_ */
