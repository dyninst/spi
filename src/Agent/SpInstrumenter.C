#include "SpInstrumenter.h"
#include "SpSnippet.h"
#include "SpContext.h"
#include "SpAddrSpace.h"
#include "SpUtils.h"
#include "SpPoint.h"

using dt::Address;

using sp::SpSnippet;
using sp::SpContext;
using sp::SpSnippet;
using sp::SpAddrSpace;
using sp::SpInstrumenter;

using in::Instruction;

using ph::Point;
using ph::Snippet;
using ph::AddrSpace;
using ph::PatchBlock;
using ph::PatchObject;
using ph::PatchMgrPtr;
using ph::InstancePtr;
using ph::PatchFunction;
using ph::PushBackCommand;

namespace sp {

  extern SpContext* g_context;

  // -------------------------------------------------------------------
  // SpInstrumenter
  // -------------------------------------------------------------------
  SpInstrumenter*
  SpInstrumenter::create(AddrSpace* as) {
    return new SpInstrumenter(as);
  }

  SpInstrumenter::SpInstrumenter(AddrSpace* as)
    : Instrumenter(as) {
#ifndef SP_RELEASE
    sp_debug("INSTRUMENTER - created");
#endif

    if (getenv("SP_TEST_SPRING") == NULL &&
        getenv("SP_TEST_RELOCBLK") == NULL &&
				getenv("SP_TEST_TRAP") == NULL) {
			// sp_print("RelocCallInsn");
      workers_.push_back(new RelocCallInsnWorker);
    }

    if (getenv("SP_TEST_SPRING") == NULL &&
				getenv("SP_TEST_TRAP") == NULL) {
			// sp_print("RelocCallBlock");
      workers_.push_back(new RelocCallBlockWorker);
    }

		if (getenv("SP_TEST_TRAP") == NULL) {
			workers_.push_back(new SpringboardWorker);
		}

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
      PushBackCommand* command = static_cast<PushBackCommand*>(*c);
      if (!command) continue;

      SpPoint* spt = static_cast<SpPoint*>(command->instance()->point());
			sp_debug("INST POINT - for call insn %lx", spt->block()->last());
      // If we only want to instrument direct call, and this point is a
			// indirect call point, then skip it
      if (spt && !spt->getCallee() && g_context->directcall_only()) {
#ifndef SP_RELEASE
        sp_debug("SKIP INDIRECT CALL");
#endif
        continue;
      }

      // If this point is already instrumented, skip it
      if (spt->instrumented()) {
        // sp_debug("SKIP INSTRUMENTED POINT");
        continue;
      }

      // Handle tail call
      Instruction::Ptr callinsn = spt->block()->getInsn(spt->block()->last());
			assert(callinsn);
			/*
			char* insn_buf = (char*)spt->block()->last();
			sp_debug("CALL INSN %s (%d)",
							 g_context->parser()->dump_insn((void*)spt->block()->last(),
							 spt->block()->end() - spt->block()->last()).c_str(),
							 callinsn->getCategory());
			sp_debug("API INSN %s",
							 g_context->parser()->dump_insn((void*)callinsn->ptr(),
																							callinsn->size()).c_str());
			sp_debug("leading code: %x", insn_buf[0]);
			*/
      if (callinsn->getCategory() == in::c_BranchInsn) {
				// XXX
#ifndef SP_RELEASE
				sp_debug("TAIL CALL");
#endif
				if (getenv("SP_NO_TAILCALL"))	{
					continue;
				}
        spt->set_tailcall(true);
        spt->set_ret_addr(0);
      } else {
        spt->set_ret_addr(spt->block()->end());
      }

      // Associate snippet with SpPoint
      Snippet<SpSnippet::ptr>::Ptr snip =
        Snippet<SpSnippet::ptr>::get(command->instance()->snippet());
      SpSnippet::ptr sp_snip = snip->rep();
      spt->set_snip(sp_snip);

      // Otherwise, apply workers one by one in order
      // If trap only, we only apply the last worker -- trap worker
      size_t i = g_context->trap_only() ? (workers_.size() - 1) : 0;
      for (; i < workers_.size(); i++) {
				// sp_print("# %ld working", i);
        InstWorker* worker = workers_[i];

        // If we cannot successfully save original instructions, it is
        // very dangous when we want to restore in the future.
				// If this worker succeeds, then we are done for current point
        if (worker->save(spt)) {
					if (worker->run(spt)) {
						spt->set_instrumented(true);
						spt->set_install_method(worker->install_method());
						++success_count;
						break; // escape the worker loop
					} else {
						sp_debug("FAILED TO INSTALL - for call insn %lx",
										 spt->block()->last());
					} // install
        } else {
					sp_debug("FAILED TO SAVE - for call insn %lx", spt->block()->last());
				} // save
      } // workers
    } // commands

		sp_debug("BATCH DONE - %d / %lu succeeded", success_count,
						 (unsigned long)user_commands_.size());
		user_commands_.clear();

    return true;
  }

  bool SpInstrumenter::undo() {
    // assert(0 && "TODO");
    return true;
  }

  // -------------------------------------------------------------------
  // TrapWorker
  // -------------------------------------------------------------------
  TrapWorker::InstMap TrapWorker::inst_map_;

  bool
  TrapWorker::run(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("TRAP WORKER - runs");
#endif
    assert(pt);

    // Install trap handler to the trap signal
    struct sigaction act;
    act.sa_sigaction = TrapWorker::trap_handler;
    act.sa_flags = SA_SIGINFO;
    struct sigaction old_act;
    sigaction(SIGTRAP, &act, &old_act);

    // Call insn's addr
    Address call_addr = pt->block()->last();

    // This mapping is used in trap handler
    inst_map_[call_addr] = pt->snip();

    // Install
    return install(pt);
  }

  bool
  TrapWorker::undo(SpPoint* pt) {
    return true;
  }

  bool
  TrapWorker::save(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("TRAP WORKER - saves");
#endif
		// sp_print("Trap Save");

    Address call_insn = pt->block()->last();
    Instruction::Ptr callinsn = pt->block()->getInsn(call_insn);
    if (!callinsn) return false;
    pt->set_orig_call_insn(callinsn);
    return true;
  }

  bool
  TrapWorker::install(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("TRAP WORKER - installs");
#endif

    SpParser::ptr parser = g_context->parser();
#ifndef SP_RELEASE
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)pt->block()->size(),
						 (unsigned long)pt->block()->last());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
						 pt->block()->end() - pt->block()->start()).c_str());
    sp_debug("}");
#endif

    char* call_addr = (char*)pt->block()->last();
    char* blob = pt->snip()->blob();
    if (!blob) {
			sp_debug("FAILED BLOB - failed to generate blob for call insn %lx",
							 (unsigned long)call_addr);
			return false;
		}

    char int3 = (char)0xcc;
    size_t call_size = pt->block()->end() - pt->block()->last();

    // Overwrite int3 to the call site
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    PatchMgrPtr mgr = g_context->parser()->mgr();
    SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(mgr->as());
    PatchObject* obj = pt->block()->object();
    if (!as->set_range_perm((Address)call_addr, call_size, perm)) {
      sp_debug("FAILED PERM - failed to change memory permission");
      return false;
    } else {
      as->write(obj, (Address)call_addr, (Address)&int3, 1);
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)call_addr, call_size)) {
			sp_debug("FAILED RESTORE - failed to restore perm for call insn %lx",
							 (unsigned long)call_addr);
      return false;
    }

#ifndef SP_RELEASE
    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)pt->block()->size(),
						 (unsigned long)pt->block()->last());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
  	pt->block()->last() - pt->block()->start() +1).c_str());
    sp_debug("}");

    sp_debug("TRAP INSTALLED - successful for call insn %lx",
						 (long)call_addr);
#endif
    return true;
  }

  // We resort to trap to transfer control to patch area, when we are not
  // able to use jump-based implementation.
  void
  TrapWorker::trap_handler(int sig, siginfo_t* info, void* c) {
    Address pc = SpSnippet::get_pre_signal_pc(c) - 1;
    InstMap& inst_map = TrapWorker::inst_map_;
    if (inst_map.find(pc) == inst_map.end()) {

      sp_perror("NO BLOB - for this call insn at %lx", pc);
      // sp_print("NO BLOB - for this call insn at %lx", pc);
    }

    // Get patch area's address
    SpSnippet::ptr sp_snip = inst_map[pc];
    char* blob = (char*)sp_snip->buf();
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    PatchMgrPtr mgr = g_context->parser()->mgr();
    SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(mgr->as());

		sp_debug("TRAP HANDLER - for call insn %lx", pc);

    // Change memory permission for the snippet
    if (!as->set_range_perm((Address)blob, sp_snip->size(), perm)) {
      // as->dump_mem_maps();
      sp_perror("FAILED PERM - failed to change memory permission for blob");
    }

		sp_debug("JUMP TO BLOB - at %lx", (Address)blob);

    // Set pc to jump to patch area
    SpSnippet::set_pc((Address)blob, c);
  }

  // -------------------------------------------------------------------
  // RelocCallInsnWorker
  // -------------------------------------------------------------------
  bool
  RelocCallInsnWorker::run(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("RELOC CALLINSN WORKER - runs");
#endif

    // Check if we are able to overwrite the call insn w/ a short jmp,
    // where we check two things:

    Address call_insn_addr = pt->block()->last();
    char* call_insn = (char*)call_insn_addr;
		size_t call_insn_size = pt->block()->end() - call_insn_addr;

		// 1. is the instruction >= 5 bytes?
		if (call_insn_size < 5) {
#ifndef SP_RELEASE
			sp_debug("SMALL CALL INSN - call insn is too small (< 5-byte),"
							 " try other workers");
#endif
			return false;
		}

    // 2. is it a direct call instruction

    if (call_insn[0] != (char)0xe8) {  // A direct call?
			sp_debug("NOT DIRECT CALL");
			if (!pt->tailcall()) {             // A jump for direct tail call?
#ifndef SP_RELEASE
				sp_debug("NOT TAIL CALL - try other workers");
#endif
				return false;
			} else {
#ifndef SP_RELEASE
				sp_debug("IS TAIL CALL");
#endif
			}
    }

    // 3. is the relative address to snippet within 4-byte?

    long rel_addr = (long)pt->snip()->buf() - (long)call_insn_addr;
    if (!sp::is_disp32(rel_addr)) {
#ifndef SP_RELEASE
      sp_debug("NOT 4-byte DISP - try other workers");
#endif
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
  RelocCallInsnWorker::save(SpPoint* pt) {

#ifndef SP_RELEASE
    sp_debug("RELOC_CALL_INSN WORKER - saves");
#endif
    Address call_insn = pt->block()->last();
    Instruction::Ptr callinsn = pt->block()->getInsn(call_insn);
    if (!callinsn) return false;
    pt->set_orig_call_insn(callinsn);
    return true;
  }

  bool
  RelocCallInsnWorker::install(SpPoint* pt) {
    // Generate the snippet
    // XXX: should we move it to the SpInstrumenter::run() ?
    char* call_addr = (char*)pt->block()->last();
    char* blob = pt->snip()->blob();
    size_t blob_size = pt->snip()->size();
    if (!blob) return false;

    char jump[5];
    memset(jump, 0, 5);
    char* p = jump;
    *p++ = 0xe9;
    // For relative address to snippet
    int* lp = (int*)p;

    SpParser::ptr parser = g_context->parser();
		assert(pt->callee());
#ifndef SP_RELEASE
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx for %s- {",
             (unsigned long)pt->block()->size(), (unsigned long)call_addr,
             pt->callee()->name().c_str());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
                                     pt->block()->size()).c_str());
    sp_debug("}");
#endif
    // Build the jump instruction
    PatchObject* obj = pt->block()->object();
    size_t insn_length = pt->block()->end() - pt->block()->last();
    *lp = (int)((long)blob - (long)call_addr - insn_length);
#ifndef SP_RELEASE
    sp_debug("REL CAL - blob at %lx, last insn at %lx, insn length %lu",
             (unsigned long)blob, pt->block()->end(), (unsigned long)insn_length);
    sp_debug("JUMP REL - jump to relative address %x", *lp);
#endif
    // Replace "call" with a "jump" instruction
    SpAddrSpace* as =
      static_cast<SpAddrSpace*>(parser->mgr()->as());

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (as->set_range_perm((Address)call_addr, insn_length, perm)) {
      as->write(obj, (Address)call_addr, (Address)jump, 5);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Change the permission of snippet, so that it can be executed.
    if (!as->set_range_perm((Address)blob, blob_size, perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (Address)blob);
      // as->dump_mem_maps();
      exit(0);
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)call_addr, insn_length)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

		assert(pt->callee());
#ifndef SP_RELEASE
    sp_debug("AFTER INSTALL (%lu bytes) for point %lx for %s- {",
             pt->block()->size(), pt->block()->last(),
             pt->callee()->name().c_str());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
                                     pt->block()->size()).c_str());
    sp_debug("}");
#endif
    return true;
  }


  // -------------------------------------------------------------------
  // RelocCallBlockWorker
  // -------------------------------------------------------------------
  bool
  RelocCallBlockWorker::run(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("RELOC CALLBLOCK WORKER - runs");
#endif

    // Assumption:
    // This worker must be run after RelocCallInsnWorker.
    // Only when RelocCallInsnWorker fails, can we use this worker.


    return install(pt);
  }

  bool
  RelocCallBlockWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  RelocCallBlockWorker::save(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("RELOC CALLBLOCK WORKER - saves");
#endif

    Address call_insn = pt->block()->last();
    Instruction::Ptr callinsn = pt->block()->getInsn(call_insn);
    if (!callinsn) return false;
    pt->set_orig_call_insn(callinsn);

    if (!pt->block()) return false;
    pt->set_orig_call_blk(pt->block());
    return true;
  }

  bool
  RelocCallBlockWorker::install(SpPoint* pt) {
    Address call_blk_addr = pt->block()->start();

    // Try to install short jump
    long rel_addr = (long)pt->snip()->buf() - (long)call_blk_addr - 5;
    char insn[64];    // the jump instruction to overwrite call blk

    if (sp::is_disp32(rel_addr)) {
#ifndef SP_RELEASE
      sp_debug("4-byte DISP - install a short jump");
#endif

      // Generate a short jump to store in insn[64]
      char* p = insn;
      *p++ = 0xe9;
      long* lp = (long*)p;
      *lp = rel_addr;

      return install_jump_to_block(pt, insn, 5);
    }

    // Try to install long jump
    if (pt->block()->size() >= pt->snip()->jump_abs_size()) {

#ifndef SP_RELEASE
      sp_debug("> 4-byte DISP - install a long jump");
#endif

      // Generate a long jump to store in insn[64]
      size_t insn_size = pt->snip()->emit_jump_abs((long)pt->snip()->buf(),
                                                   insn, 0, true);

      return install_jump_to_block(pt, insn, insn_size);
    }

    // Well, let's try spring board next ...
#ifndef SP_RELEASE
    sp_debug("FAILED RELOC BLK - try other worker");
#endif
    return false;
  }

  bool RelocCallBlockWorker::install_jump_to_block(SpPoint* pt,
                                                   char* jump_insn,
                                                   size_t insn_size) {
    SpParser::ptr parser = g_context->parser();

#ifndef SP_RELEASE
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             pt->block()->size(), pt->block()->last());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
                                     pt->block()->size()).c_str());
    sp_debug("}");
#endif
		assert(pt);
		assert(pt->snip());
    // Build blob & change the permission of snippet
    char* blob = pt->snip()->blob(/*reloc=*/true);
		if (!blob) return false;

    PatchObject* obj = pt->block()->obj();
    SpAddrSpace* as = static_cast<SpAddrSpace*>(parser->mgr()->as());
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (!as->set_range_perm((Address)blob, pt->snip()->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (Address)blob);
      // as->dump_mem_maps();
      exit(0);
    }

    char* addr = (char*)pt->block()->start();

    // Write a jump instruction to call block
    if (as->set_range_perm((Address)addr, insn_size, perm)) {
      as->write(obj, (Address)addr, (Address)jump_insn, insn_size);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, insn_size)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }
#ifndef SP_RELEASE
    sp_debug("USE BLK-RELOC - piont %lx is instrumented using call"
             " block relocation", pt->block()->last());

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             pt->block()->size(), pt->block()->last());
    sp_debug("%s", parser->dump_insn((void*)pt->block()->start(),
                                     pt->block()->size()).c_str());
    sp_debug("}");
#endif

    return true;
  }


  // -------------------------------------------------------------------
  // SpringboardWorker
  // -------------------------------------------------------------------
  bool
  SpringboardWorker::run(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("SPRINGBOARD WORKER - runs");
#endif
    return install(pt);
  }

  bool
  SpringboardWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  SpringboardWorker::save(SpPoint* pt) {
#ifndef SP_RELEASE
    sp_debug("SPRING WORKER - saves");
#endif
    PatchBlock* springblk = pt->snip()->spring_blk();
    if (!springblk) {
#ifndef SP_RELEASE
      sp_debug("NO SPRING BOARD - cannot find suitable spring board");
#endif
      return false;
    }
    pt->set_orig_spring(springblk);

    Address call_insn = pt->block()->last();
    Instruction::Ptr callinsn = pt->block()->getInsn(call_insn);
    if (!callinsn) return false;
    pt->set_orig_call_insn(callinsn);

    if (!pt->block()) return false;
    pt->set_orig_call_blk(pt->block());

    return true;
  }

  bool
  SpringboardWorker::install(SpPoint* pt) {
    SpParser::ptr parser = g_context->parser();
    PatchBlock* callblk = pt->block();
    SpSnippet::ptr snip = pt->snip();
#ifndef SP_RELEASE
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", parser->dump_insn((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");
#endif
    // Find a spring block. A spring block should:
    // - big enough to hold two absolute jumps
    // - close enough to short jump from call block
    // if we cannot find one available spring block, just use trap or
    // ignore this call
    PatchBlock* springblk = snip->spring_blk();
    if (!springblk) return false;

#ifndef SP_RELEASE
    sp_debug("BEFORE INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", parser->dump_insn((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");
#endif
    // Build blob & change the permission of snippet
    char* blob = snip->blob(/*reloc=*/true, /*spring=*/true);
    PatchObject* obj = callblk->obj();
    SpAddrSpace* as = static_cast<SpAddrSpace*>(parser->mgr()->as());

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (!as->set_range_perm((Address)blob, snip->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (Address)blob);
      // as->dump_mem_maps();
      exit(0);
    }


    // Relocate spring block & change the permission of the relocated
    // spring block
    char* spring = snip->spring(springblk->last());
    obj = springblk->obj();
    if (!as->set_range_perm((Address)spring, snip->spring_size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for relocated spring blk at %lx", (Address)spring);
      // as->dump_mem_maps();
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
    if (as->set_range_perm((Address)addr, springblk->size(), perm)) {
      as->write(obj, (Address)addr, (Address)springblk_insn, off);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, springblk->size())) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    // Handle call block
    // Write a "jump" instruction to call block
    addr = (char*)callblk->start();
    char callblk_insn[2];
    callblk_insn[0] = 0xeb;
    callblk_insn[1] = (char)(springblk->start() + call_blk_jmp_trg
                             - ((long)addr + 2));

    if (as->set_range_perm((Address)addr, callblk->size(), perm)) {
      as->write(obj, (Address)addr, (Address)callblk_insn, 2);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, callblk->size())) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

#ifndef SP_RELEASE
    sp_debug("AFTER INSTALL CALL BLK (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", parser->dump_insn((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");

    sp_debug("AFTER INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", parser->dump_insn((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");

    sp_debug("USE SPRING - piont %lx is instrumented using 1-hop spring",
             callblk->last());
#endif

    return true;
  }

} // namespace sp

