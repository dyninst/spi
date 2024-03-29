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

#include "agent/patchapi/object.h"
#include "common/common.h"

namespace sp {
	extern SpAddrSpace* g_as;

	void SpObject::InitMemoryAlloc(dt::Address base,
                                 size_t size) {
		sp_debug_patchapi("INIT MEMORY ALLOC - base %lx, size %ld",
             (long)base, (long)size);

		// MMap this big buffer
		void* m = MAP_FAILED;
		m = mmap((void*)base,
						 size,
						 PROT_WRITE | PROT_READ | PROT_EXEC,
						 MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
						 0,
						 0);

		while(m == MAP_FAILED) {
			base += getpagesize();
			size -= getpagesize();
			if ((long)size < 0) break;
			m = mmap((void*)base,
							 size,
							 PROT_WRITE | PROT_READ | PROT_EXEC,
							 MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
							 0,
							 0);
		}

		if (m == MAP_FAILED) {
			sp_debug_patchapi("%s: FAILED TO MAP TO %lx (size %ld)",
							 name().c_str(), (long)base, (long)size);
			return;
		} else {
			sp_debug_patchapi("%s: SUCCEED TO MAP TO %lx (size %ld)",
							 name().c_str(), (long)m, (long)size);
		}

		small_freebufs_.base =base;
		small_freebufs_.buf_size = size ;

	}

	dt::Address
	SpObject::AllocateBuffer(size_t size) {
		dt::Address ret = 0;

		if (small_freebufs_.buf_size < size) {
			// not enough space in the memory pool
			// we need to ask AddressSpace for a new interval
			if (g_as == NULL) {
				sp_perror("NULL pointer to AddrSpace when asking for a new interval allocation");
				return 0;
			}
			g_as->allocateNewInterval(this);
		}

		if (small_freebufs_.buf_size >= size) {
			ret = small_freebufs_.base;
			//sp_debug_patchapi("Number of free buffers: %d", small_freebufs_.list.size());
			sp_debug_patchapi("%s: Returning size %lu buffer at %lx from interval [%lx, %lx]", name().c_str(), size, ret, small_freebufs_.base, (small_freebufs_.base + small_freebufs_.buf_size));
			small_freebufs_.base += (size+1);
			small_freebufs_.buf_size -= (size+1);
			sp_debug_patchapi("%s: %lu bytes left in interval [%lx, %lx]", name().c_str(), small_freebufs_.buf_size, small_freebufs_.base, (small_freebufs_.base + small_freebufs_.buf_size));
			return ret;
		}
		size_t ps = getpagesize();
		size = ((size + ps -1) & ~(ps - 1));
		if (::posix_memalign((void**)&ret, ps, size) == 0) {
			// sp_print("FAILED TO GET A CLOSE BUFFER - %lx malloced", ret);
			sp_debug_patchapi("FAILED TO GET A CLOSE BUFFER - %lu at %lx malloced", size, ret);
				return ret;
		}
		sp_debug_patchapi("FAILED TO GET A CLOSE BUFFER - 0 is malloced");
			return 0;
	}

	bool
	SpObject::FreeBuffer(dt::Address buf) {
    if (!buf) return false;
    
		if (alloc_bufs_.find(buf) == alloc_bufs_.end()) {
			sp_debug_patchapi("FREE FROM MALLOC-ed - %lx is allocated by malloc", buf);
      ::free((void*)buf);
      buf = (dt::Address)NULL;
      return true;
    }

		BufType type = alloc_bufs_[buf];
		switch (type) {
		case SMALL_BUF:
			small_freebufs_.list.push_back(buf);
			break;
		default:
      sp_perror("UNKNOWN FREE - %lx", buf);
			assert(0);
		}
		alloc_bufs_.erase(buf);
		return true;
	}
}
