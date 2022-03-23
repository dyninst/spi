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

#include "agent/context.h"
#include "agent/inst_workers/callblk_worker_impl.h"
#include "agent/inst_workers/callinsn_worker_impl.h"
#include "agent/inst_workers/inst_worker_delegate.h"
#include "agent/inst_workers/spring_worker_impl.h"
#include "agent/inst_workers/trap_worker_impl.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/instrumenter.h"
#include "agent/patchapi/object.h"
#include "agent/patchapi/point.h"
#include "agent/snippet.h"
#include "common/utils.h"


namespace sp {

  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

  SpInstrumenter*
  SpInstrumenter::create(ph::AddrSpace* as) {
    return new SpInstrumenter(as);
  }

  SpInstrumenter::SpInstrumenter(ph::AddrSpace* as)
    : ph::Instrumenter(as) {

    sp_debug_patchapi("INSTRUMENTER - created");

		// Relocate insn
		if (getenv("SP_TEST_RELOCINSN")) {
			sp_debug_patchapi("ONLY TEST RELOCINSN WORKER");
			workers_.push_back(new RelocCallInsnWorker);
		}

		// Relocate call block
		if (getenv("SP_TEST_RELOCBLK")) {
			sp_debug_patchapi("ONLY TEST RELOCBLK WORKER");
			workers_.push_back(new RelocCallBlockWorker);
		}

		// Only use springboard
		if (getenv("SP_TEST_SPRING")) {
			sp_debug_patchapi("ONLY TEST SPRING BOARD WORKER");
			workers_.push_back(new SpringboardWorker);
		}

		// Only use trap
		if (getenv("SP_TEST_TRAP")) {
			sp_debug_patchapi("ONLY TEST TRAP WORKER");
			workers_.push_back(new TrapWorker);
		}

		if (getenv("SP_TEST_RELOCINSN") ||
				getenv("SP_TEST_RELOCBLK") ||
				getenv("SP_TEST_SPRING") ||
				getenv("SP_TEST_TRAP")) {
			sp_debug_patchapi("DEBUGGING MODE - test a subset of workers");
			return;
		}

		// Use combination of all workers
		workers_.push_back(new RelocCallInsnWorker);
		workers_.push_back(new RelocCallBlockWorker);
		workers_.push_back(new SpringboardWorker);
		workers_.push_back(new TrapWorker);
  }

  SpInstrumenter::~SpInstrumenter() {
    for (InstWorkers::iterator i = workers_.begin();
         i != workers_.end(); i++) {
      delete *i;
    }
  }

  bool
  SpInstrumenter::run() {

		int success_count = 0;

    // Do all callees in a function in a batch
    for (CommandList::iterator c = user_commands_.begin();
         c != user_commands_.end(); c++) {
			ph::PushBackCommand* command =
          static_cast<ph::PushBackCommand*>(*c);
      
      if (!command) {
				sp_debug_patchapi("BAD COMMAND - skip");
				continue;
			}

      SpPoint* spt = PT_CAST(command->instance()->point());
			assert(spt);
			SpBlock* blk = spt->GetBlock();
			assert(blk);
			sp_debug_patchapi("INSTRUMENTING POINT - for call insn %lx", blk->last());

      // If we only want to instrument direct call, and this point is a
			// indirect call point, then skip it
      if (spt &&
          !spt->getCallee() &&
          g_context->IsDirectcallOnlyEnabled()) {
        sp_debug_patchapi("INDIRECT CALL - skip");
        continue;
      }

			// We should have already escaped instrumented points in Propeller
			// If not, something wrong happens
			assert(!blk->instrumented());

      // Handle tail call
			in::Instruction callinsn = blk->getInsn(blk->last());
			assert(callinsn.ptr());

      if (callinsn.getCategory() == in::c_BranchInsn) {
				sp_debug_patchapi("TAIL CALL - for call insn %lx", blk->last());
				if (getenv("SP_NO_TAILCALL"))	{
					continue;
				}
        spt->SetIsTailcall(true);
        spt->SetRetAddr(0);
      } else {
        spt->SetRetAddr(blk->end());
      }

      // Otherwise, apply workers one by one in order
      for (size_t i = 0; i < workers_.size(); i++) {
        InstWorkerDelegate* worker = workers_[i];
				assert(worker);

        // If we cannot successfully save original instructions, it is
        // very dangous when we want to restore in the future.
				// If this worker succeeds, then we are done for current point
        if (worker->save(spt)) {
					sp_debug_patchapi("SAVE SUCCESSFULLY");
					if (worker->run(spt)) {
						blk->SetInstrumented(true);
						spt->SetInstallMethod(worker->install_method());
						++success_count;
						break; // escape the worker loop
					} else {
						sp_debug_patchapi("FAILED TO INSTALL - for call insn %lx",
										 blk->last());
					} // install
        } else {
					sp_debug_patchapi("FAILED TO SAVE - for call insn %lx", blk->last());
				} // save
      } // workers
    } // commands

		sp_debug_patchapi("BATCH DONE - %d / %lu succeeded", success_count,
						 (unsigned long)user_commands_.size());
		user_commands_.clear();

    return true;
  }

  bool SpInstrumenter::undo() {
    return true;
  }


} // namespace sp

