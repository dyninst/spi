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

namespace sp {

// Parse /proc/pid/maps file to build memory mappings
void
SpParser::UpdateMemoryMappings() {

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
SpParser::DumpMemoryMappings() {

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
SpParser::UpdateFreeIntervals() {
  
}

void
SpParser::DumpFreeIntervals() {
  for (FreeIntervalList::iterator i = free_intervals_.begin();
       i != free_intervals_.end(); i++) {
    sp_debug("FREE INTERVAL - [%lx ~ %lx], used %d",
             (*i).start, (*i).end, (*i).used);
  }
}

}
