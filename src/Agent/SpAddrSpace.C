#include "SpAddrSpace.h"
#include "SpObject.h"

using dt::Address;
using sp::SpAddrSpace;
using ph::PatchObject;

namespace sp {
/* Build mapped memory area. 
   TODO (wenbin): should update mapped memory area dynamically
*/
SpAddrSpace::SpAddrSpace() : ph::AddrSpace() {
  update_mem_maps();
  //dump_mem_maps();
}

/* Standard PatchAPI stuff ... */
SpAddrSpace*
SpAddrSpace::create(PatchObject* obj) {
  assert(obj);
  SpAddrSpace* ret = new SpAddrSpace;
  if (!ret) return ret;
  ret->init(obj);
  return ret;
}

/* Memory allocator, which is maily to allocate a buffer for the patch area. */
Address
SpAddrSpace::malloc(PatchObject* obj, size_t size, Address near) {
  Address buf = 0;
  buf = (Address)::malloc(size+ getpagesize() -1);
  if (!buf) return 0;
  buf = (((Address)buf + getpagesize()-1) & ~(getpagesize()-1));
  return buf;

  /* TODO (wenbin): should have a better memory allocation subsystem */
  Address ps = getpagesize();
  Address r_near = ((near + ps -1) & ~(ps - 1));
  Address r_size = ((size + ps -1) & ~(ps - 1));
  buf = r_near;

  // sp_print("page size: %d, near: %lx, r_near: %lx, size: %d, r_size: %d",
  //         ps, near, r_near, size, r_size);
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
  return (Address)m;
}

/* Copy a buffer to another buffer. */
bool
SpAddrSpace::write(PatchObject* obj, Address to, Address from, size_t size) {
  return (memcpy((void*)to, (void*)from, size) == (void*)to);
}

/* Deallocate a buffer. */
bool
SpAddrSpace::free(PatchObject* obj, Address orig) {
  /* TODO (wenbin): should implement uninstrumentation and deallocation
                    of patch area.
  */
  ::free((void*)orig);
  return true;
}

/* Set access permission to a memory area. */
bool
SpAddrSpace::set_range_perm(Address a, size_t length, int perm) {
  bool ret = false;

  /* Check the mapped memory area.
     We change permission for the entire range that overlaps what we provide
     in the argument list.
     TODO (wenbin): safer to only change a minimal range of area's permission ...
  */
  for (MemMappings::iterator mi = mem_maps_.begin();
       mi != mem_maps_.end(); mi++) {
    Address start = mi->first;
    MemMapping& mm = mi->second;
    Address end = mm.end;
    Address code_end = a + length;
    if ((a >= start && code_end < end) ||
        (a <= start && code_end >= start && code_end <= end) ||
        (a >= start && a < end && code_end >= end)) {
      sp_debug("PERM - [%lx, %lx) overlaps (%lx, %lx)", start, end, a, code_end);
      if (mprotect((void*)start, end - start, perm) < 0) {
        sp_print("MPROTECT - Failed to change memory access permission");
        perror("mprotect");
        return false;
      } else {
        ret = true;
      }
    }
  }

  /* Last resort to align the address ... */
  if (!ret) {
    Address aligned = a;
    size_t pz = getpagesize();
    aligned = (Address)(((Address) aligned + pz-1) & ~(pz-1));
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
SpAddrSpace::restore_range_perm(Address a, size_t length) {
  bool ret = false;
  for (MemMappings::iterator mi = mem_maps_.begin();
       mi != mem_maps_.end(); mi++) {
    Address start = mi->first;
    MemMapping& mm = mi->second;
    Address end = mm.end;
    Address code_end = a + length;
    if ((a >= start && code_end < end) ||
        (a <= start && code_end >= start && code_end <= end) ||
        (a >= start && a < end && code_end >= end)) {

      if (mprotect((void*)start, end - start, mem_maps_[start].perms) < 0) {
        sp_print("MPROTECT - Failed to change memory access permission");
        return false;
      } else {
        ret = true;
      }
    }
  }
  return ret;
}

/* Parse /proc/pid/maps file to build memory mappings */
void
SpAddrSpace::update_mem_maps() {

  char maps_file[256];
  sprintf(maps_file, "/proc/%d/maps", getpid());

  FILE* fp = fopen(maps_file, "r");
  if (!fp) {
    sp_perror("FAILED to open memory mapping file %s", maps_file);
  }
  char linebuf[2048];
  while (fgets(linebuf, 2048, fp) != NULL) {
    char* start_addr_s = linebuf;
    char* end_addr_s = strchr(linebuf, '-');
    *end_addr_s = '\0';
    end_addr_s++;

    char* perms_s = strchr(end_addr_s, ' ');
    *perms_s = '\0';
    perms_s++;

    char* offset_s = strchr(perms_s, ' ');
    *offset_s = '\0';
    offset_s++;

    char* dev_s = strchr(offset_s, ' ');
    *dev_s = '\0';
    dev_s++;

    char* inode_s = strchr(dev_s, ' ');
    *inode_s = '\0';
    inode_s++;

    char* pch = strtok(inode_s, " \n\r");
    inode_s = pch;
    char* path_s = NULL;
    if (pch != NULL) {
      pch = strtok(NULL, " \n\r");
      if (pch) {
        path_s = pch;
      }
    }

    char* pDummy;
    Address start = strtoll(start_addr_s, &pDummy, 16);


    if (mem_maps_.find(start) == mem_maps_.end()) {
      MemMapping& mapping = mem_maps_[start];
      mapping.start = start;
      mapping.end = strtol(end_addr_s, &pDummy, 16);
      mapping.offset = strtol(offset_s, &pDummy, 16);
      mapping.dev = dev_s;
      mapping.inode = strtol(inode_s, &pDummy, 16);
      if (path_s) mapping.path = path_s;
      int perms = 0;
      int count = 0;
      if (perms_s[0] == 'r') {
        perms |= PROT_READ;
        ++count;
      }
      if (perms_s[1] == 'w') {
        perms |= PROT_WRITE;
        ++count;
      }
      if (perms_s[2] == 'x') {
        perms |= PROT_EXEC;
        ++count;
      }
      if (count == 3) perms = PROT_NONE;
      mapping.perms = perms;
    }
  }
  fclose(fp);
}

void
SpAddrSpace::dump_mem_maps() {
#ifndef SP_RELEASE
  sp_debug("MMAPS - %lu memory mappings", mem_maps_.size());
#endif
  for (MemMappings::iterator mi = mem_maps_.begin(); mi != mem_maps_.end(); mi++) {
#ifndef SP_RELEASE
    MemMapping& mapping = mi->second;
    sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s, Inode %lu, Path %s",
             mapping.start, mapping.end, mapping.offset, mapping.perms,
             mapping.dev.c_str(), mapping.inode, mapping.path.c_str());
#endif
  }
}

void SpAddrSpace::loadLibrary(ph::PatchObject* obj) {
  loadObject(obj);
  pre_alloc_near(static_cast<SpObject*>(obj));
}

/* Preallocate a set of 1KB buffers near an object.

  Current strategy:
  Before an object, allocate 100 1KB buffers.
*/
void
SpAddrSpace::pre_alloc_near(sp::SpObject* obj) {
  return;
  /* Initialize some key variables */
  buf_size_ = 1024;
  max_buf_num_ = 10;
  size_t max_size = max_buf_num_ * buf_size_;

  Address ps = getpagesize();
  Address r_near = ((obj->load_addr() + ps -1) & ~(ps - 1));
  Address r_size = ((max_size + ps -1) & ~(ps - 1));
  Address buf = r_near;

  update_mem_maps();
  dump_mem_maps();
  sp_print("codeBase: %lx, r_near: %lx, r_size: %lu", obj->codeBase(), r_near, r_size);
  //  while(1);
  void* m = NULL;
  int fd = -1;
  while (!m || (long)m == -1) {
    buf -= r_size;
    //    sp_print("looking for %lx", buf);
    m = mmap((void*)buf,
	     r_size,
	     PROT_WRITE | PROT_READ,
	     MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
	     fd,
	     0);
    //    sp_print("pass mmap");
  }
  if (!m) {
    /* Something bad happen... Skip it for now. */
    // sp_print("WARNING: cannot prealloc buffer before %lx", obj->load_addr());
  } else {
    sp_print("get buf: %lx", (Dyninst::Address)m);
  }
}

/* Get a chunk near this point. */
char*
SpAddrSpace::get_near(ph::Point* pt) {
	return NULL;
}

}
