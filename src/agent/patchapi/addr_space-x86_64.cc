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
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"

namespace sp {

// Parse /proc/pid/maps file to build memory mappings
void
SpAddrSpace::UpdateMemoryMappings() {

  char maps_file[256];
  sprintf(maps_file, "/proc/%d/maps", getpid());

  FILE* fp = fopen(maps_file, "r");
  if (!fp) {
    sp_perror("FAILED to open memory mapping file %s", maps_file);
  }

  char linebuf[2048];
  dt::Address previous_end = 0;

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
    dt::Address start = strtoll(start_addr_s, &pDummy, 16);


    if (mem_maps_.find(start) == mem_maps_.end()) {
      MemMapping& mapping = mem_maps_[start];
      mapping.previous_end = previous_end;
      mapping.start = start;
      mapping.end = strtol(end_addr_s, &pDummy, 16);
      mapping.offset = strtol(offset_s, &pDummy, 16);
      mapping.dev = dev_s;
      mapping.inode = strtol(inode_s, &pDummy, 16);

      if (previous_end != mapping.start) {
        FreeInterval interval;
        interval.start = previous_end;
        interval.end = mapping.start;
        interval.used = false;
        free_intervals_.push_back(interval);
      }

      previous_end = mapping.end;
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
SpAddrSpace::DumpMemoryMappings() {

  sp_debug("MMAPS - %lu memory mappings",
           (unsigned long)mem_maps_.size());

  for (MemMappings::iterator mi = mem_maps_.begin();
       mi != mem_maps_.end(); mi++) {

    MemMapping& mapping = mi->second;
    sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s,"
             " Inode %lu, Path %s, previous_end %lx",
             mapping.start, mapping.end, mapping.offset,
             mapping.perms, mapping.dev.c_str(), mapping.inode,
             mapping.path.c_str(),
             mapping.previous_end);
  }
}

void
SpAddrSpace::UpdateFreeIntervals() {

  if (getenv("SP_LIBC_MALLOC")) {
    return;
  }

  
  // Re-arrange free buffers. Some rules:
  // - Reduce each "hole" between two mapped objects to less than 1.5GB
  // - Each interval should have size smaller than 16MB

  FreeIntervalList tmp_list;
  std::copy(free_intervals_.begin(), free_intervals_.end(),
            back_inserter(tmp_list));
  free_intervals_.clear();
  const size_t max_interval_size = (const size_t)1024*1024*16;
  const size_t max_mapped_area_size = (const size_t)1024*1024*(1024+512);
  for (FreeIntervalList::iterator fi = tmp_list.begin();
       fi != tmp_list.end(); fi++) {
    if ((*fi).size() > max_mapped_area_size) {
      (*fi).start = (*fi).end - max_mapped_area_size;
    }
  }
    
  for (FreeIntervalList::iterator fi = tmp_list.begin();
       fi != tmp_list.end(); fi++) {
    dt::Address new_start = (*fi).start;
    dt::Address new_end = new_start + max_interval_size;
      
    do {
      FreeInterval new_interval;
      new_interval.used = false;
      new_interval.start = new_start;
      new_interval.end = new_end;
      if (new_start + max_interval_size > (*fi).end) {
        new_interval.end = (*fi).end;
      }
      free_intervals_.push_back(new_interval);
      new_start += max_interval_size;
      new_end += max_interval_size;
    } while(new_start < (*fi).end);
  }

  // Bind preallocated close free buffers to each object
  for (ph::AddrSpace::ObjMap::iterator i = obj_map_.begin();
       i != obj_map_.end(); i++) {

    SpObject* obj = OBJ_CAST(i->second);
    assert(obj);
    sp_debug("HANDLING OBJECT - %s @ load addr: %lx, code base: %lx",
             obj->name().c_str(), obj->load_addr(), obj->codeBase());
    MemMapping& mapping = mem_maps_[obj->load_addr()];
    sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s,"
             " Inode %lu, Path %s, previous_end %lx",
             mapping.start, mapping.end, mapping.offset,
             mapping.perms, mapping.dev.c_str(), mapping.inode,
             mapping.path.c_str(),
             mapping.previous_end);

    FreeInterval* interval = NULL;
    if (!GetClosestInterval(mapping.start, &interval)) {
      sp_debug("FAILED TO GET FREE INTERVAL - for %lx %s",
               mapping.start, obj->name().c_str());
      continue;
    }
    assert(interval);
    size_t size = interval->size();
    size_t ps = getpagesize();

    sp_debug("GET FREE INTERVAL - [%lx, %lx], w/ original size %ld, "
             "rounded size %ld", (long)interval->start,
             (long)interval->end, (long)interval->size(), (long)size);

    size = (size <= 2147483646 ? size : 2147483646);
    size = ((size + ps -1) & ~(ps - 1));

    dt::Address base = interval->end - size;
    if (base < ps) {
      base += ps;
      size -= ps;
    }
    obj->InitMemoryAlloc(base, size);
  }
}

void
SpAddrSpace::DumpFreeIntervals() {
  for (FreeIntervalList::iterator i = free_intervals_.begin();
       i != free_intervals_.end(); i++) {
    sp_debug("FREE INTERVAL - [%lx ~ %lx], used %d, size: %ld MB",
             (*i).start, (*i).end, (*i).used, (*i).size()/1024/1024);
  }
}

bool
SpAddrSpace::allocateNewInterval(SpObject* obj) {
  if (obj == NULL) {
    sp_perror("NULL pointer to SpObject when trying to allocate a new interval for object");
    return false;
  }
  sp_debug("Trying to allocate new interval for object %s", obj->name().c_str());

  sp_debug("HANDLING OBJECT - %s @ load addr: %lx, code base: %lx",
             obj->name().c_str(), obj->load_addr(), obj->codeBase());
  MemMapping& mapping = mem_maps_[obj->load_addr()];
  sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s,"
            " Inode %lu, Path %s, previous_end %lx",
            mapping.start, mapping.end, mapping.offset,
            mapping.perms, mapping.dev.c_str(), mapping.inode,
            mapping.path.c_str(),
            mapping.previous_end);

  FreeInterval* interval = NULL;
  if (!GetClosestInterval(mapping.start, &interval)) {
    sp_debug("FAILED TO GET FREE INTERVAL - for %lx %s",
              mapping.start, obj->name().c_str());
    return false;
  }
  assert(interval);
  size_t size = interval->size();
  size_t ps = getpagesize();

  sp_debug("GET FREE INTERVAL - [%lx, %lx], w/ original size %ld, "
            "rounded size %ld", (long)interval->start,
            (long)interval->end, (long)interval->size(), (long)size);

  size = (size <= 2147483646 ? size : 2147483646);
  size = ((size + ps -1) & ~(ps - 1));

  dt::Address base = interval->end - size;
  if (base < ps) {
    base += ps;
    size -= ps;
  }
  obj->InitMemoryAlloc(base, size);
  return true;
}

bool
SpAddrSpace::GetClosestInterval(dt::Address addr,
                                FreeInterval** interval) {
  const size_t distance = (const size_t)(1024*1024*(1024+512));
  
  for (FreeIntervalList::iterator i = free_intervals_.begin();
       i != free_intervals_.end(); i++) {
    FreeInterval* previous = &(*i);

    // Bind a free interval before this object, and the distance should
    // be shorter than 1.5GB
    sp_debug("GET CLOSEST INTERVAL - free interval [%lx, %lx], size %ld",
             (*i).start, (*i).end, (*i).size());
    if ((*i).start > addr) {
      sp_debug("start larger than addr");
      continue;
    } else {
      if (!previous) return false;
      if (previous->used) {
        sp_debug("this interval already used");
        continue;
      }
      if ((addr - (*i).start) > distance) {
        sp_debug("distance too far");
        continue;
      }
      *interval = previous;
      (*interval)->used = true;
      return true;
    }
  }
  return false;
}

}
