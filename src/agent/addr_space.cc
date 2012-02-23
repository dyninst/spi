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

#include "agent/addr_space.h"
#include "agent/object.h"
#include "agent/parser.h"

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

		// Allocate a nearby buffer
		dt::Address buf = o->get_freebuf(size);
		if (buf) {
			sp_debug("SUCCESSFUL TO GET A CLOSE BUFFER to %lx - %lx allocaed",
							 near, buf);
			return buf;
		}

		// If the above effort fails, we resort to malloc
    buf = (dt::Address)::malloc(size);
		assert(buf);
		sp_debug("FAILED TO GET A CLOSE BUFFER %lx - %lx malloced", near, buf);
    return buf;
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
		SpObject* o = static_cast<SpObject*>(obj);
		assert(o);
		if (!o->put_freebuf(orig)) {
			sp_debug("FREE FROM MALLOC-ed - %lx is allocated by malloc", orig);
			::free((void*)orig);
		}
    return true;
  }

  // Set access permission to a original code area.
  bool
  SpAddrSpace::SetCodePermission(dt::Address a,
                                 size_t length,
                                 int perm) {
    bool ret = false;
    sp_debug("SETTING CODE PERM - for [%lx ~ %lx]", a, a + length - 1);

    // Checks the mapped memory area.
    // We change permission for the entire range that overlaps what we
		// provide in the argument list.
		MemMappings& mem_maps = g_parser->mem_maps();
    for (MemMappings::iterator mi = mem_maps.begin();
         mi != mem_maps.end(); mi++) {
      dt::Address start = mi->first;
      MemMapping& mm = mi->second;
      dt::Address end = mm.end;
      dt::Address code_end = a + length;
      if ((a >= start && code_end < end) ||
          (a <= start && code_end >= start && code_end <= end) ||
          (a >= start && a < end && code_end >= end)) {

        sp_debug("PERM - [%lx, %lx) overlaps (%lx, %lx)",
								 start, end, a, code_end);

        if (mprotect((void*)start, end - start, perm) < 0) {
          sp_debug("MPROTECT - FAILED TO change memory access"
                   " perm for %lx in [%lx, %lx]", a, start, end);
          perror("mprotect");
          return false;
        } else {
          sp_debug("MPROTECT - SUCCEED TO change memory access"
                   " perm for %lx in [%lx, %lx]", a, start, end);
          ret = true;
        }
      }
    }

    return ret;
  }


  // Set access permission to snippet buffer area.
  bool
  SpAddrSpace::SetSnippetPermission(dt::Address a,
                                    size_t length,
                                    int perm) {
    bool ret = false;
    sp_debug("SETTING SNIPPET PERM - for [%lx ~ %lx]", a, a + length - 1);

    dt::Address aligned = a;
    size_t pz = getpagesize();
    aligned = (dt::Address)(((dt::Address) aligned - pz-1) & ~(pz-1));
    size_t len = length + pz + pz;
    if (mprotect((void*)aligned, len, perm) < 0) {
      sp_print("MPROTECT - Failed to change memory access permission");
      perror("mprotect");
      return false;
    } else {
      sp_debug("MPROTECT - SUCCEED TO change memory access"
               " perm for %lx in [%lx, %lx]", a, aligned, aligned+len-1);
      ret = true;
    }

    return ret;
  }

  // Restore original code's access permission
  // XXX: is it really necessary? to be safe?
  bool
  SpAddrSpace::RestoreCodePermission(dt::Address a,
                                     size_t length) {
    bool ret = false;
		MemMappings& mem_maps = g_parser->mem_maps();
    for (MemMappings::iterator mi = mem_maps.begin();
         mi != mem_maps.end(); mi++) {
      dt::Address start = mi->first;
      MemMapping& mm = mi->second;
      dt::Address end = mm.end;
      dt::Address code_end = a + length;
      if ((a >= start && code_end < end) ||
          (a <= start && code_end >= start && code_end <= end) ||
          (a >= start && a < end && code_end >= end)) {

        if (mprotect((void*)start, end - start, mem_maps[start].perms) < 0) {
          sp_print("MPROTECT - Failed to change memory access permission");
          return false;
        } else {
          ret = true;
        }
      }
    }
    return ret;
  }

}
