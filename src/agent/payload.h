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

#ifndef SP_PAYLOAD_H_
#define SP_PAYLOAD_H_

#include "agent/patchapi/cfg.h"
#include "common/common.h"

// Wrappers for locking
#define SP_LOCK(func) do {                              \
    int result = Lock(&g_propel_lock);                  \
    if (result == SP_DEAD_LOCK) {                       \
      sp_print("DEAD LOCK - skip #func for point %lx",  \
               pt->block()->last());                    \
      goto func##_EXIT;                                 \
    }                                                   \
  } while (0)

#define SP_UNLOCK(func) do {                    \
 func##_EXIT:                                   \
    Unlock(&g_propel_lock);                     \
  } while (0)

namespace sp {
  class SpContext;
  class SpPoint;

  // a struct to pass information tuple between payload entry and payload exit
  struct PointCallInfo {
    PointCallInfo(SpPoint* pt, SpFunction* callee, void* info) {
      this->pt = pt;
      this->callee = callee;
      this->info = info;
    }

    SpPoint* pt;
    SpFunction* callee;
    void* info;
  };

  class PointHandle {
    SpPoint* pt_;
    SpFunction* callee_;
    void* user_info_;
    long return_value_;

  public:
    PointHandle(SpPoint*, SpFunction*, void*, long);
    ~PointHandle();

    SpPoint* GetPoint();
    SpFunction* GetCallee();
    void* GetUserInfo();
    long ReturnValue();
  };

  // ------------------------
  //       Private things
  // ------------------------
  typedef void (*PayloadFunc_t)(ph::Point* pt);
  typedef void* (*PayloadFuncEntry)(ph::Point* pt);
  typedef void (*PayloadFuncExit)(PointHandle*); 
  typedef void* PayloadFunc;
  struct ArgumentHandle {
    ArgumentHandle();
    ~ArgumentHandle();
    char* insert_buf(size_t s);

    long offset;
    long num;
    std::vector<char*> bufs;
  };
  void wrapper_before(SpPoint* pt,
                      PayloadFunc_t before);
  void wrapper_after(SpPoint* pt,
                     PayloadFunc_t before);

  // ------------------------
  //       Public things
  // ------------------------

  // Only used in before_payload
  AGENT_EXPORT void Propel(SpPoint* pt_);
  AGENT_EXPORT void* PopArgument(SpPoint* pt,
                                 ArgumentHandle* h,
                                 size_t size);

	// Implicitly call StartTracing()
  AGENT_EXPORT bool IsIpcWrite(SpPoint*); 
  AGENT_EXPORT bool IsIpcRead(SpPoint*); 

  // Only used in after_payload
  AGENT_EXPORT long ReturnValue(SpPoint* pt);

  // Used in both payloads
  AGENT_EXPORT SpFunction* Callee(SpPoint* pt);
  AGENT_EXPORT char StartTracing(SpPoint* pt,
                                 int fd);
  AGENT_EXPORT bool IsInstrumentable(SpPoint* pt);
}

#endif /* SP_PAYLOAD_H_ */
