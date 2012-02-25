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

#include <sys/mman.h>

#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"

namespace sp {
	extern SpParser::ptr g_parser;

  // Build mapped memory area.
  // TODO (wenbin): should update mapped memory area dynamically
  SpAddrSpace::SpAddrSpace()
		: ph::AddrSpace() {
  }

  // Standard PatchAPI stuff ...
  SpAddrSpace*
  SpAddrSpace::Create(ph::PatchObject* obj) {
    assert(obj);
    SpAddrSpace* ret = new SpAddrSpace;
    if (!ret) {
			sp_perror("FAILED TO CREATE SpAddrSpace");
			return ret;
		}
    ret->init(obj);
    return ret;
  }

  // Memory allocator, which is maily to allocate a buffer for the
	// patch area.
  dt::Address
  SpAddrSpace::malloc(ph::PatchObject* obj,
											size_t size,
											dt::Address near) {
		SpObject* o = OBJ_CAST(obj);
		assert(o);
		return o->AllocateBuffer(size);
  }

  // Copy a buffer to another buffer.
  bool
  SpAddrSpace::write(ph::PatchObject* obj,
										 dt::Address to,
										 dt::Address from,
										 size_t size) {
    return (memcpy((void*)to, (void*)from, size) == (void*)to);
  }

  // Deallocate a buffer.
  bool
  SpAddrSpace::free(ph::PatchObject* obj,
										dt::Address orig) {
		SpObject* o = OBJ_CAST(obj);
		assert(o);
		return o->FreeBuffer(orig);
  }


  // Set access permission to memory area.
  bool
  SpAddrSpace::SetMemoryPermission(dt::Address a,
                                    size_t length,
                                    int perm) {
    bool ret = false;
    sp_debug("SETTING MEMORY PERM - for [%lx ~ %lx] of %ld bytes",
             (long)a, (long)(a + length - 1), (long)length);

    dt::Address aligned = a;
    size_t pz = getpagesize();
    if (a > pz) {
      sp_debug("PAGE SIZE SMALLER - pagesize %lx, address %lx", pz, a);
      aligned = (dt::Address)(((dt::Address) aligned - pz-1) & ~(pz-1));
    } else if (a % pz == 0) {
      aligned = a;
    } else {
      sp_debug("PAGE SIZE LARGER - pagesize %lx, address %lx", pz, a);
      aligned = 0;
    }

    size_t len = length + (a - aligned); 
    sp_debug("TRY mprotect - for [%lx ~ %lx] of %ld bytes",
             (long)aligned, (long)(aligned + len - 1), (long)len);
    if (mprotect((void*)aligned, len, perm) < 0) {
      sp_debug("MPROTECT - Failed to change memory access permission");
      // perror("mprotect");
      return false;
    } else {
      sp_debug("MPROTECT - SUCCEED TO change memory access"
               " perm for %lx in [%lx, %lx]", a, aligned, aligned+len-1);
      ret = true;
    }

    return ret;
  }

  void
  SpAddrSpace::InitMemoryAllocator() {
    UpdateMemoryMappings();
    UpdateFreeIntervals();
    DumpMemoryMappings();
    DumpFreeIntervals();
  }
}
