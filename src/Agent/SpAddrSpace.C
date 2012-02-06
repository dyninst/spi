#include "SpAddrSpace.h"
#include "SpObject.h"
#include "SpParser.h"

namespace sp {
	extern SpParser::ptr g_parser;

  // Build mapped memory area.
  // TODO (wenbin): should update mapped memory area dynamically
  SpAddrSpace::SpAddrSpace()
		: ph::AddrSpace() {
    //update_mem_maps();
    //dump_mem_maps();
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
    dt::Address buf = 0;
    buf = (dt::Address)::malloc(size+ getpagesize() -1);
    if (!buf) return 0;
    buf = (((dt::Address)buf + getpagesize()-1) & ~(getpagesize()-1));
    return buf;

    // TODO (wenbin): should have a better memory allocation subsystem
    dt::Address ps = getpagesize();
    dt::Address r_near = ((near + ps -1) & ~(ps - 1));
    dt::Address r_size = ((size + ps -1) & ~(ps - 1));
    buf = r_near;
    void* m = NULL;
    int fd = -1;
    while (!m || (long)m == -1) {
      buf -= r_size;
      sp_print("looking for %lx", buf);
      m = mmap((void*)buf,
               r_size,
               PROT_WRITE | PROT_READ,
               MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
               fd,
               0);
    }
    if (!m) return 0;
    // sp_print("get buf: %lx", m);
    return (dt::Address)m;
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
    // TODO (wenbin): should implement uninstrumentation and deallocation
    // of patch area.
    ::free((void*)orig);
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
