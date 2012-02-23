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

#include "agent/cfg.h"
#include "common/common.h"

namespace sp {
  class SpContext;
  class SpPoint;

  // ------------------------
  //       Private things
  // ------------------------
  typedef void (*PayloadFunc_t)(ph::Point* pt);
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
  AGENT_EXPORT void propel(SpPoint* pt_);
  AGENT_EXPORT void* pop_argument(SpPoint* pt,
                                  ArgumentHandle* h,
                                  size_t size);

	// Implicitly call start_tracing()
  AGENT_EXPORT bool is_ipc_write(SpPoint*); 
  AGENT_EXPORT bool is_ipc_read(SpPoint*); 

  // Only used in after_payload
  AGENT_EXPORT long retval(SpPoint* pt);

  // Used in both payloads
  AGENT_EXPORT SpFunction* callee(SpPoint* pt_);
  AGENT_EXPORT char start_tracing(int fd);

}

#endif /* SP_PAYLOAD_H_ */
