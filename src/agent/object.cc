#include <sys/mman.h>

#include "agent/object.h"
#include "common/common.h"

namespace sp {

  // Get the object's name
  std::string SpObject::name() {
		assert(symtab_);
    return symtab_->name();
  }

	void SpObject::init_memory_alloc(dt::Address base, size_t size) {
		sp_debug("INIT MEMORY ALLOC - base %lx, size %ld", base, size);

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
							 base, size);
			return;
		} else {
			sp_debug("SUCCEED TO MAP TO %lx (size %ld)",
							 (long)m, size);
		}

		const double small_buf_num_ratio = 0.8;
		const size_t small_buf_total_size_limit =
			(size_t)(small_buf_num_ratio * size);
		const size_t small_buf_size = 128;

		const double mid_buf_num_ratio = 0.15;
		size_t mid_buf_total_size_limit =
			(size_t)(mid_buf_num_ratio * size);
		const size_t mid_buf_size = 256;

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
						 small_freebufs_.list.size(),
						 small_freebufs_.base,
						 small_freebufs_.buf_size * small_freebufs_.list.size(),
						 small_freebufs_.buf_size);

		// For midium free bufs
		mid_freebufs_.base = base;
		mid_freebufs_.buf_size = mid_buf_size;

		mid_buf_total_size_limit += offset;
		for (; offset < mid_buf_total_size_limit; offset += mid_buf_size) {
			mid_freebufs_.list.push_back(offset);
		}
		offset -= mid_buf_size;

		sp_debug("MID BUF (total %ld) - base %lx, total_size %ld, buf_size %ld",
						 mid_freebufs_.list.size(),
						 mid_freebufs_.base,
						 mid_freebufs_.buf_size * mid_freebufs_.list.size(),
						 mid_freebufs_.buf_size);


		// For big free bufs
		big_freebufs_.base = base;
		big_freebufs_.buf_size = big_buf_size;

		for (; offset < size; offset += big_buf_size) {
			big_freebufs_.list.push_back(offset);
		}
		offset -= big_buf_size;

		sp_debug("BIG BUF (total %ld) - base %lx, total_size %ld, buf_size %ld",
						 big_freebufs_.list.size(),
						 big_freebufs_.base,
						 big_freebufs_.buf_size * big_freebufs_.list.size(),
						 big_freebufs_.buf_size);

	}

	dt::Address
	SpObject::get_freebuf(size_t size) {
		dt::Address ret = 0;

		if (small_freebufs_.list.size() > 0 &&
				small_freebufs_.buf_size >= size) {
			ret = small_freebufs_.list.front() + small_freebufs_.base;
			alloc_bufs_[ret] = SMALL_BUF;
			small_freebufs_.list.pop_front();
		}

		if (mid_freebufs_.list.size() > 0 &&
				mid_freebufs_.buf_size >= size) {
			ret =	mid_freebufs_.list.front() + mid_freebufs_.base;
			alloc_bufs_[ret] = MID_BUF;
			mid_freebufs_.list.pop_front();
		}

		if (big_freebufs_.list.size() > 0 &&
				big_freebufs_.buf_size >= size) {
			ret =	big_freebufs_.list.front() + big_freebufs_.base;
			alloc_bufs_[ret] = BIG_BUF;
			big_freebufs_.list.pop_front();
		}

		return ret;
	}

	bool
	SpObject::put_freebuf(dt::Address buf) {
		if (alloc_bufs_.find(buf) == alloc_bufs_.end()) return false;

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
