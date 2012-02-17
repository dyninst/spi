#include "SpAddrSpace.h"
#include "SpObject.h"
#include "SpParser.h"

namespace sp {
	extern SpParser::ptr g_parser;

  // Build mapped memory area.
  // TODO (wenbin): should update mapped memory area dynamically
  SpAddrSpace::SpAddrSpace()
		: ph::AddrSpace() {
  }

  // Standard PatchAPI stuff ...
  SpAddrSpace*
  SpAddrSpace::create(ph::PatchObject* obj) {
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

  // Set access permission to a memory area.
  bool
  SpAddrSpace::set_range_perm(dt::Address a,
															size_t length,
															int perm) {
    bool ret = false;

    // Check the mapped memory area.
    // We change permission for the entire range that overlaps what we
		// provide in the argument list.
    // TODO (wenbin): safer to only change a minimal range of area's
		//                permission ...
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
#ifndef SP_RELEASE
        sp_debug("PERM - [%lx, %lx) overlaps (%lx, %lx)",
								 start, end, a, code_end);
#endif
        if (mprotect((void*)start, end - start, perm) < 0) {
          sp_print("MPROTECT - Failed to change memory access permission");
          perror("mprotect");
          return false;
        } else {
          ret = true;
        }
      }
    }

    // Last resort to align the address ...
    if (!ret) {
      dt::Address aligned = a;
      size_t pz = getpagesize();
      aligned = (dt::Address)(((dt::Address) aligned + pz-1) & ~(pz-1));
      if (mprotect((void*)aligned, length, perm) < 0) {
        sp_print("MPROTECT - Failed to change memory access permission");
        perror("mprotect");
        return false;
      } else {
        ret = true;
      }
    }

    return ret;
  }

  bool
  SpAddrSpace::restore_range_perm(dt::Address a,
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

  void SpAddrSpace::loadLibrary(ph::PatchObject* obj) {
    loadObject(obj);
    // pre_alloc_near(OBJ_CAST(obj));
  }

}
