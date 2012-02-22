#include <sys/mman.h>

#include "common/utils.h"

#include "agent/point.h"
#include "agent/object.h"
#include "agent/snippet.h"
#include "agent/addr_space.h"
#include "agent/context.h"
#include "agent/instrumenter.h"
#include "agent/inst_worker_delegate.h"
#include "agent/trap_worker_impl.h"
#include "agent/callinsn_worker_impl.h"
#include "agent/callblk_worker_impl.h"
#include "agent/spring_worker_impl.h"

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

    sp_debug("INSTRUMENTER - created");

		// Relocate insn
		if (getenv("SP_TEST_RELOCINSN")) {
			sp_debug("ONLY TEST RELOCINSN WORKER");
			workers_.push_back(new RelocCallInsnWorker);
		}

		// Relocate call block
		if (getenv("SP_TEST_RELOCBLK")) {
			sp_debug("ONLY TEST RELOCBLK WORKER");
			workers_.push_back(new RelocCallBlockWorker);
		}

		// Only use springboard
		if (getenv("SP_TEST_SPRING")) {
			sp_debug("ONLY TEST SPRING BOARD WORKER");
			workers_.push_back(new SpringboardWorker);
		}

		// Only use trap
		if (getenv("SP_TEST_TRAP")) {
			sp_debug("ONLY TEST TRAP WORKER");
			workers_.push_back(new TrapWorker);
		}

		if (getenv("SP_TEST_RELOCINSN") ||
				getenv("SP_TEST_RELOCBLK") ||
				getenv("SP_TEST_SPRING") ||
				getenv("SP_TEST_TRAP")) {
			sp_debug("DEBUGGING MODE - test a subset of workers");
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
			ph::PushBackCommand* command = static_cast<ph::PushBackCommand*>(*c);
      if (!command) {
				sp_debug("BAD COMMAND - skip");
				continue;
			}

      SpPoint* spt = static_cast<SpPoint*>(command->instance()->point());
			assert(spt);
			SpBlock* blk = spt->get_block();
			assert(blk);
			sp_debug("INSTRUMENTING POINT - for call insn %lx", blk->last());

      // If we only want to instrument direct call, and this point is a
			// indirect call point, then skip it
      if (spt && !spt->getCallee() && g_context->directcall_only()) {
        sp_debug("INDIRECT CALL - skip");
        continue;
      }

			// We should have already escaped instrumented points in Propeller
			// If not, something worong happens
			assert(!blk->instrumented());

      // Handle tail call
			in::Instruction::Ptr callinsn = blk->getInsn(blk->last());
			assert(callinsn);

      if (callinsn->getCategory() == in::c_BranchInsn) {
				sp_debug("TAIL CALL - for call insn %lx", blk->last());
				if (getenv("SP_NO_TAILCALL"))	{
					continue;
				}
        spt->set_tailcall(true);
        spt->set_ret_addr(0);
      } else {
        spt->set_ret_addr(blk->end());
      }

      // Otherwise, apply workers one by one in order
      // If trap only, we only apply the last worker -- trap worker

      for (size_t i = 0; i < workers_.size(); i++) {
        InstWorkerDelegate* worker = workers_[i];
				assert(worker);

        // If we cannot successfully save original instructions, it is
        // very dangous when we want to restore in the future.
				// If this worker succeeds, then we are done for current point
        if (worker->save(spt)) {
					sp_debug("SAVE SUCCESSFULLY");
					if (worker->run(spt)) {
						blk->set_instrumented(true);
						spt->set_install_method(worker->install_method());
						++success_count;
						break; // escape the worker loop
					} else {
						sp_debug("FAILED TO INSTALL - for call insn %lx",
										 blk->last());
					} // install
        } else {
					sp_debug("FAILED TO SAVE - for call insn %lx", blk->last());
				} // save
      } // workers
    } // commands

		sp_debug("BATCH DONE - %d / %lu succeeded", success_count,
						 (unsigned long)user_commands_.size());
		user_commands_.clear();

    return true;
  }

  bool SpInstrumenter::undo() {
    return true;
  }


} // namespace sp

