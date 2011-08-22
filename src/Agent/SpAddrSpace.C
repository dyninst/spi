#include <sys/mman.h>

#include "SpAddrSpace.h"
#include "SpCommon.h"
#include "PatchObject.h"

using sp::SpAddrSpace;
using Dyninst::PatchAPI::PatchObject;

SpAddrSpace::SpAddrSpace()
 : Dyninst::PatchAPI::AddrSpace() {

  update_mem_maps();
  dump_mem_maps();

}

SpAddrSpace::ptr SpAddrSpace::create(PatchObject* obj) {
  ptr ret = ptr(new SpAddrSpace);
  if (!ret) return ret;
  ret->init(obj);
  return ret;
}


Dyninst::Address SpAddrSpace::malloc(PatchObject* obj,
                                     size_t size,
                                     Dyninst::Address /*near*/) {
  Dyninst::Address addr = obj->codeBase();
  void* m = NULL;

  size_t ps = getpagesize();
  size_t adjust_size = ((size + ps) & (~ps));
  assert(adjust_size % ps == 0);
  while (!m && addr > 0) {
    addr -= adjust_size;
    m = mmap((void*)addr,
             adjust_size,
             PROT_WRITE | PROT_READ | PROT_EXEC,
             MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
             -1,
             0);
  }
  if (addr) return (Dyninst::Address)m;

  return 0;
}

bool SpAddrSpace::write(PatchObject* obj, Dyninst::Address to,
                        Dyninst::Address from, size_t size) {
  return false;
}

bool SpAddrSpace::free(PatchObject* obj, Dyninst::Address orig) {
  return false;
}

bool SpAddrSpace::set_range_perm(Dyninst::Address a, size_t length, int perm) {
  // Find [a, length] in range
  // Set perm
  return false;
}

bool SpAddrSpace::restore_range_perm(Dyninst::Address a, size_t length) {
  // Find [a, length] in range
  // Restore perm
  return false;
}

// Parse /proc/pid/maps file to build memory mappings
void SpAddrSpace::update_mem_maps() {
  sp_debug("UPDATE - Memory mappings");
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
    Dyninst::Address start = strtol(start_addr_s, &pDummy, 16);
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

void SpAddrSpace::dump_mem_maps() {
  sp_debug("MMAPS - %d memory mappings", mem_maps_.size());
  for (MemMappings::iterator mi = mem_maps_.begin(); mi != mem_maps_.end(); mi++) {
    MemMapping& mapping = mi->second;
    sp_debug("MMAP - Range[%x ~ %x], Offset %x, Perm %x, Dev %s, Inode %d, Path %s",
             mapping.start, mapping.end, mapping.offset, mapping.perms, mapping.dev.c_str(),
             mapping.inode, mapping.path.c_str());
  }
}

