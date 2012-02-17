#include "SpUtils.h"
#include "SpPoint.h"
#include "SpObject.h"
#include "SpSnippet.h"
#include "SpContext.h"
#include "SpAddrSpace.h"
#include "SpInstrumenter.h"

namespace sp {

  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

  // -------------------------------------------------------------------
  // SpInstrumenter
  // -------------------------------------------------------------------
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
        InstWorker* worker = workers_[i];
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

  // -------------------------------------------------------------------
  // InstWorker: base class
  // -------------------------------------------------------------------
	bool InstWorker::save(SpPoint* pt) {
		assert(pt);
		assert(pt->get_block());
		return pt->get_block()->save();
	}

  // -------------------------------------------------------------------
  // TrapWorker
  // -------------------------------------------------------------------

	// Used in trap handler, for mapping call instruction's address to snippet
  TrapWorker::InstMap TrapWorker::inst_map_;

  bool
  TrapWorker::run(SpPoint* pt) {
    sp_debug("TRAP WORKER - runs");
    assert(pt);

    // Install trap handler to the trap signal
    struct sigaction act;
    act.sa_sigaction = TrapWorker::trap_handler;
    act.sa_flags = SA_SIGINFO;
    struct sigaction old_act;
    sigaction(SIGTRAP, &act, &old_act);

    // Call insn's addr
		assert(pt->get_block());
		dt::Address call_addr = pt->get_block()->last();

    // This mapping is used in trap handler
		assert(pt->snip());
    inst_map_[call_addr] = pt->snip();

    // Install
    return install(pt);
  }

  bool
  TrapWorker::undo(SpPoint* pt) {
    return true;
  }

  bool
  TrapWorker::install(SpPoint* pt) {
		assert(pt);

		SpBlock* b = pt->get_block();
		assert(b);
		
    sp_debug("TRAP WORKER - installs");

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)b->size(),
						 (unsigned long)b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    char* call_addr = (char*)b->last();
		assert(call_addr);

    char* blob = pt->snip()->blob();
    if (!blob) {
			sp_debug("FAILED BLOB - failed to generate blob for call insn %lx",
							 (unsigned long)call_addr);
			return false;
		}

    char int3 = (char)0xcc;
    size_t call_size = b->call_size();

    // Overwrite int3 to the call site
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		SpObject* obj = pt->get_object();
		assert(obj);
		assert(g_as);

    if (!g_as->set_range_perm((dt::Address)call_addr, call_size, perm)) {
      sp_debug("FAILED PERM - failed to change memory permission");
      return false;
    } else {
      g_as->write(obj, (dt::Address)call_addr, (dt::Address)&int3, 1);
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)call_addr, call_size)) {
			sp_debug("FAILED RESTORE - failed to restore perm for call insn %lx",
							 (unsigned long)call_addr);
      return false;
    }

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)b->size(),
						 (unsigned long)b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->last() - b->start() +1).c_str());
    sp_debug("}");

    sp_debug("TRAP INSTALLED - successful for call insn %lx",
						 (long)call_addr);
    return true;
  }

  // We resort to trap to transfer control to patch area, when we are not
  // able to use jump-based implementation.
  void
  TrapWorker::trap_handler(int sig, siginfo_t* info, void* c) {
    dt::Address pc = SpSnippet::get_pre_signal_pc(c) - 1;
		assert(pc);

    InstMap& inst_map = TrapWorker::inst_map_;
    if (inst_map.find(pc) == inst_map.end()) {
      sp_perror("NO BLOB - for this call insn at %lx", pc);
    }

    // Get patch area's address
    SpSnippet::ptr sp_snip = inst_map[pc];
		assert(sp_snip);

    char* blob = (char*)sp_snip->buf();
		assert(sp_snip);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		sp_debug("TRAP HANDLER - for call insn %lx", pc);

		assert(g_as);
    // Change memory permission for the snippet
    if (!g_as->set_range_perm((dt::Address)blob, sp_snip->size(), perm)) {
      // g_as->dump_mem_maps();
      sp_perror("FAILED PERM - failed to change memory permission for blob");
    }

		sp_debug("JUMP TO BLOB - at %lx", (dt::Address)blob);

    // Set pc to jump to patch area
    SpSnippet::set_pc((dt::Address)blob, c);
  }

  // -------------------------------------------------------------------
  // RelocCallInsnWorker
  // -------------------------------------------------------------------
  bool
  RelocCallInsnWorker::run(SpPoint* pt) {
    sp_debug("RELOC CALLINSN WORKER - runs");

		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    // Check if we are able to overwrite the call insn w/ a short jmp,
    // where we check two things:

    dt::Address call_insn_addr = b->last();
    char* call_insn = (char*)call_insn_addr;
		assert(call_insn);

		size_t call_insn_size = b->call_size();

		// 1. is the instruction >= 5 bytes?
		if (call_insn_size < 5) {
			sp_debug("SMALL CALL INSN - call insn is too small (< 5-byte),"
							 " try other workers");
			return false;
		}

    // 2. is it a direct call instruction
    if (call_insn[0] != (char)0xe8) {  // A direct call?
			sp_debug("NOT NORMAL CALL");
			if (!pt->tailcall()) {           // A jump for direct tail call?
				sp_debug("NOT TAIL CALL - try other workers");
				return false;
			} else {
				sp_debug("IS TAIL CALL");
				assert(call_insn_size == 5 &&
							 "LARGE INDIRECT TAIL CALL, UNSUPPORTED");
			}
    }

    // 3. is the relative address to snippet within 4-byte?
		assert(pt->snip());
		assert(pt->snip()->buf());
    long rel_addr = (long)pt->snip()->buf() - (long)call_insn_addr;
    if (!sp::is_disp32(rel_addr)) {
      sp_debug("NOT 4-byte DISP - try other workers");
      return false;
    }

    return install(pt);
  }

  bool
  RelocCallInsnWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  RelocCallInsnWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    // Generate the snippet
    // XXX: should we move it to the SpInstrumenter::run() ?
    char* call_addr = (char*)b->last();
		assert(call_addr);

    char* blob = pt->snip()->blob();
    size_t blob_size = pt->snip()->size();
    if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}

    char jump[5];
    memset(jump, 0, 5);
    char* p = jump;
    *p++ = 0xe9;
    // For relative address to snippet
    int* lp = (int*)p;

		assert(pt->callee());
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx for %s- {",
             (unsigned long)b->size(), (unsigned long)call_addr,
             pt->callee()->name().c_str());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    // Build the jump instruction
		SpObject* obj = pt->get_object();
		assert(obj);

    size_t insn_length = b->call_size();
    *lp = (int)((long)blob - (long)call_addr - insn_length);

    sp_debug("REL CAL - blob at %lx, last insn at %lx, insn length %lu",
             (unsigned long)blob, b->end(),
						 (unsigned long)insn_length);
    sp_debug("JUMP REL - jump to relative address %x", *lp);

    // Replace "call" with a "jump" instruction

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (g_as->set_range_perm((dt::Address)call_addr, insn_length, perm)) {
      g_as->write(obj, (dt::Address)call_addr, (dt::Address)jump, 5);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Change the permission of snippet, so that it can be executed.
    if (!g_as->set_range_perm((dt::Address)blob, blob_size, perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)call_addr, insn_length)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

		assert(pt->callee());

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx for %s- {",
             b->size(), b->last(),
             pt->callee()->name().c_str());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    return true;
  }


  // -------------------------------------------------------------------
  // RelocCallBlockWorker
  // -------------------------------------------------------------------
  bool
  RelocCallBlockWorker::run(SpPoint* pt) {
    sp_debug("RELOC CALLBLOCK WORKER - runs");

    return install(pt);
  }

  bool
  RelocCallBlockWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }


  bool
  RelocCallBlockWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    dt::Address call_blk_addr = b->start();

    // Try to install short jump
		assert(pt->snip());
		assert(pt->snip()->buf());
    long rel_addr = (long)pt->snip()->buf() - (long)call_blk_addr - 5;
    char insn[64];    // the jump instruction to overwrite call blk

    if (sp::is_disp32(rel_addr)) {
      sp_debug("4-byte DISP - install a short jump");

      // Generate a short jump to store in insn[64]
      char* p = insn;
      *p++ = 0xe9;
      long* lp = (long*)p;
      *lp = rel_addr;

      return install_jump_to_block(pt, insn, 5);
    }

    // Try to install long jump
    if (b->size() >= pt->snip()->jump_abs_size()) {

      sp_debug("> 4-byte DISP - install a long jump");

      // Generate a long jump to store in insn[64]
      size_t insn_size = pt->snip()->emit_jump_abs((long)pt->snip()->buf(),
                                                   insn, 0, true);

      return install_jump_to_block(pt, insn, insn_size);
    } else {
      sp_debug("CALL BLK TOO SMALL - %ld < %ld", b->size(),
							 pt->snip()->jump_abs_size());
		}

    // Well, let's try spring board next ...
    sp_debug("FAILED RELOC BLK - try other worker");

    return false;
  }

  bool RelocCallBlockWorker::install_jump_to_block(SpPoint* pt,
                                                   char* jump_insn,
                                                   size_t insn_size) {

		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             b->size(), b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

		assert(pt);
		assert(pt->snip());
    // Build blob & change the permission of snippet
    char* blob = pt->snip()->blob(/*reloc=*/true);
		if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}

		SpObject* obj = pt->get_object();
		assert(obj);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (!g_as->set_range_perm((dt::Address)blob, pt->snip()->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }

    char* addr = (char*)b->start();
		assert(addr);

    // Write a jump instruction to call block
    if (g_as->set_range_perm((dt::Address)addr, insn_size, perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)jump_insn, insn_size);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)addr, insn_size)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    sp_debug("USE BLK-RELOC - piont %lx is instrumented using call"
             " block relocation", b->last());

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             b->size(), b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    return true;
  }


  // -------------------------------------------------------------------
  // SpringboardWorker
  // -------------------------------------------------------------------
  bool
  SpringboardWorker::run(SpPoint* pt) {
    sp_debug("SPRINGBOARD WORKER - runs");
    return install(pt);
  }

  bool
  SpringboardWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  SpringboardWorker::save(SpPoint* pt) {
    assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

		sp_debug("SPRING WORKER - saves");
		bool ret = false;

		assert(pt->snip());
		ph::PatchBlock* springblk = pt->snip()->spring_blk();
    if (!springblk) {
      sp_debug("NO SPRING BOARD - cannot find suitable spring board");
      return false;
    }
		SpBlock* sblk = BLK_CAST(springblk);
		assert(sblk);
		ret = sblk->save();

		assert(pt->get_block());
		ret = (ret && pt->get_block()->save());

		return ret;
  }

  bool
  SpringboardWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* callblk = pt->get_block();
		assert(callblk);

    SpSnippet::ptr snip = pt->snip();
		assert(snip);

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", g_parser->dump_insn((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");

    // Find a spring block. A spring block should:
    // - big enough to hold two absolute jumps
    // - close enough to short jump from call block
    // if we cannot find one available spring block, just use trap or
    // ignore this call
		ph::PatchBlock* springblk = snip->spring_blk();
    if (!springblk) {
			sp_debug("FAILED TO GET A SPRINGBOARD BLOCK");
			return false;
		}

    sp_debug("BEFORE INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", g_parser->dump_insn((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");

    // Build blob & change the permission of snippet
    char* blob = snip->blob(/*reloc=*/true, /*spring=*/true);
		if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}
		SpObject* obj = callblk->get_object();
		assert(obj);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (!g_as->set_range_perm((dt::Address)blob, snip->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }


    // Relocate spring block & change the permission of the relocated
    // spring block
    char* spring = snip->spring(springblk->last());
		if (!spring) {
			sp_debug("FAILED TO RELOCATE SPRINGBOARD BLOCK");
			return false;
		}
    obj = BLK_CAST(springblk)->get_object();
    if (!g_as->set_range_perm((dt::Address)spring,
															snip->spring_size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for relocated spring blk at %lx", (dt::Address)spring);
      // g_as->dump_mem_maps();
      exit(0);
    }


    // Handle spring block
    // Write two "jump" instruction to spring block
    char springblk_insn[64];

    // First jump to relocated spring block
    size_t off = 0;
    size_t isize = snip->emit_jump_abs((long)spring, springblk_insn,
                                       off, /*abs=*/true);
    off += isize;
    size_t call_blk_jmp_trg = off;
    off = snip->emit_jump_abs((long)blob, springblk_insn, off,
                              /*abs=*/true);
    off += isize;

    // Second jump to relocated spring block
    char* addr = (char*)springblk->start();
    if (g_as->set_range_perm((dt::Address)addr, springblk->size(), perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)springblk_insn, off);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)addr, springblk->size())) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    // Handle call block
    // Write a "jump" instruction to call block
    addr = (char*)callblk->start();
    char callblk_insn[2];
    callblk_insn[0] = 0xeb;
    callblk_insn[1] = (char)(springblk->start() + call_blk_jmp_trg
                             - ((long)addr + 2));

    if (g_as->set_range_perm((dt::Address)addr, callblk->size(), perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)callblk_insn, 2);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)addr, callblk->size())) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    sp_debug("AFTER INSTALL CALL BLK (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", g_parser->dump_insn((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");

    sp_debug("AFTER INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", g_parser->dump_insn((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");

    sp_debug("USE SPRING - piont %lx is instrumented using 1-hop spring",
             callblk->last());

    return true;
  }

} // namespace sp

