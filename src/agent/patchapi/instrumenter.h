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

// This defines an instrumenter that does binary instrumentation logics
// by trying different workers one by one to generate binary code.
// For current implementation:
// 1. We first try to overwrite call instruction with a short jump
// 2. If it doesn't work, we try to overwrite the call block with a
//    short jump or a long jump, depending on the size of call block
// 3. If it still doesn't work, we build one-hop spring board
// 4. If it doesn't work at all, we resort to overwrite call insn w/
//    a trap instruction

#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "agent/patchapi/point.h"
#include "agent/snippet.h"
#include "common/common.h"

#include "Instrumenter.h"

namespace sp {

	// Forward declaration
	class InstWorkerDelegate;

	class AGENT_EXPORT SpInstrumenter : public ph::Instrumenter {
  public:
    static SpInstrumenter* create(ph::AddrSpace* as);

		virtual bool run() OVERRIDE;
		virtual bool undo() OVERRIDE;

  protected:
    // For future extension, we many want more instworkers ...
		typedef std::vector<InstWorkerDelegate*> InstWorkers;
		InstWorkers workers_;

    SpInstrumenter(ph::AddrSpace* as);
		~SpInstrumenter();
	};

}
#endif /* _SPINSTRUMENTER_H_ */
