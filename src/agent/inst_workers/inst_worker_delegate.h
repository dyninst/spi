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

#ifndef _INST_WORKER_DELEGATE_H_
#define _INST_WORKER_DELEGATE_H_

#include <signal.h>

#include "agent/patchapi/point.h"

namespace sp {

	// The base class for workers
	class InstWorkerDelegate {
	public:
		// Instrument a point
		virtual bool run(SpPoint* pt) = 0;

		// Uninstrument a point
		virtual bool undo(SpPoint* pt) = 0;

		// Save code that will be modified for a point
		virtual bool save(SpPoint* pt) OVERRIDE {
      assert(pt);
      assert(pt->GetBlock());
      return pt->GetBlock()->save();
    }

		// How to install instrumentation?
		// - SP_NONE,
    // - SP_TRAP,      
    // - SP_RELOC_INSN,
    // - SP_RELOC_BLK, 
    // - SP_SPRINGBOARD
		virtual InstallMethod install_method() const = 0;

		virtual ~InstWorkerDelegate() {}
	protected:
		// Install the instrumentation
		virtual bool install(SpPoint* pt) = 0;
		virtual size_t EstimateBlobSize(SpPoint* pt) = 0;

		// The base size of generated binary code (excluding relocated insns)

		// For relocating an insn
		static size_t BaseEstimateRelocInsnSize(SpPoint* pt) {
      return 100;
    }

		// For relocating a block
		static size_t BaseEstimateRelocBlockSize(SpPoint* pt) {
      assert(pt);
      SpBlock* blk = pt->GetBlock();
      assert(blk);
      return blk->size() + InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
    }
	};


}

#endif  // _INST_WORKER_DELEGATE_H_
