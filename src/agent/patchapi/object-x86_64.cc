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

	void SpObject::InitMemoryAlloc(dt::Address base,
                                 size_t size) {
		sp_debug("INIT MEMORY ALLOC - base %lx, size %ld",
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
			sp_debug("FAILED TO MAP TO %lx (size %ld)",
							 (long)base, (long)size);
			return;
		} else {
			sp_debug("SUCCEED TO MAP TO %lx (size %ld)",
							 (long)m, (long)size);
		}

		const double small_buf_num_ratio = 0.8;
		const size_t small_buf_total_size_limit =
			(size_t)(small_buf_num_ratio * size);
		const size_t small_buf_size = 128;

		const double mid_buf_num_ratio = 0.15;
		size_t mid_buf_total_size_limit =
			(size_t)(mid_buf_num_ratio * size);
		const size_t mid_buf_size = 512;

		const size_t big_buf_size = 4096;

		// For small free bufs
		small_freebufs_.base = base;
		small_freebufs_.buf_size = small_buf_size;

		unsigned offset = 0;
		for (; offset < small_buf_total_size_limit;
				 offset += small_buf_size) {
			small_freebufs_.list.push_back(offset);
		}
		offset -= small_buf_size;

		sp_debug("SMALL BUF (total %ld)- base %lx, total_size %ld, buf_size %ld",
						 (long)small_freebufs_.list.size(),
						 (long)small_freebufs_.base,
						 (long)small_freebufs_.buf_size * small_freebufs_.list.size(),
						 (long)small_freebufs_.buf_size);

		// For midium free bufs
		mid_freebufs_.base = base;
		mid_freebufs_.buf_size = mid_buf_size;

		mid_buf_total_size_limit += offset;
		for (; offset < mid_buf_total_size_limit; offset += mid_buf_size) {
			mid_freebufs_.list.push_back(offset);
		}
		offset -= mid_buf_size;

		sp_debug("MID BUF (total %ld) - base %lx, total_size %ld, buf_size %ld",
						 (long)mid_freebufs_.list.size(),
						 (long)mid_freebufs_.base,
						 (long)mid_freebufs_.buf_size * mid_freebufs_.list.size(),
						 (long)mid_freebufs_.buf_size);


		// For big free bufs
		big_freebufs_.base = base;
		big_freebufs_.buf_size = big_buf_size;

		for (; offset < size; offset += big_buf_size) {
			big_freebufs_.list.push_back(offset);
		}
		offset -= big_buf_size;

		sp_debug("BIG BUF (total %ld) - base %lx, total_size %ld, buf_size %ld",
						 (long)big_freebufs_.list.size(),
						 (long)big_freebufs_.base,
						 (long)big_freebufs_.buf_size * big_freebufs_.list.size(),
						 (long)big_freebufs_.buf_size);

	}

	dt::Address
	SpObject::AllocateBuffer(size_t size) {
		dt::Address ret = 0;

		if (small_freebufs_.list.size() > 0 &&
				small_freebufs_.buf_size >= size) {
			ret = small_freebufs_.list.front() + small_freebufs_.base;
			alloc_bufs_[ret] = SMALL_BUF;
			small_freebufs_.list.pop_front();
      return ret;
		}

		if (mid_freebufs_.list.size() > 0 &&
				mid_freebufs_.buf_size >= size) {
			ret =	mid_freebufs_.list.front() + mid_freebufs_.base;
			alloc_bufs_[ret] = MID_BUF;
			mid_freebufs_.list.pop_front();
      return ret;
		}

		if (big_freebufs_.list.size() > 0 &&
				big_freebufs_.buf_size >= size) {
			ret =	big_freebufs_.list.front() + big_freebufs_.base;
			alloc_bufs_[ret] = BIG_BUF;
			big_freebufs_.list.pop_front();
      return ret;
		}

    // ret = (dt::Address)::malloc(size);
    if (::posix_memalign((void**)&ret, getpagesize(), size) == 0) {
      sp_debug("FAILED TO GET A CLOSE BUFFER - %lx malloced", ret);
      return ret;
    }
    sp_debug("FAILED TO GET A CLOSE BUFFER - 0 is malloced");
    return ret;
	}

	bool
	SpObject::FreeBuffer(dt::Address buf) {
		if (alloc_bufs_.find(buf) == alloc_bufs_.end()) {
			sp_debug("FREE FROM MALLOC-ed - %lx is allocated by malloc", buf);
			::free((void*)buf);
      return false;
    }

		BufType type = alloc_bufs_[buf];
		switch (type) {
		case SMALL_BUF:
			small_freebufs_.list.push_back(buf);
			break;
		case MID_BUF:
			mid_freebufs_.list.push_back(buf);
			break;
		case BIG_BUF:
			big_freebufs_.list.push_back(buf);
			break;
		default:
			assert(0);
		}
		alloc_bufs_.erase(buf);
		return true;
	}
}
