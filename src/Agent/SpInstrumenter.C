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

  // Jump-based instrumenter, which is to use a jump instruction to transfer
  // control to the patch area.
  SpInstrumenter*
  SpInstrumenter::create(AddrSpace* as) {
    return new SpInstrumenter(as);
  }


  SpInstrumenter::SpInstrumenter(AddrSpace* as)
    : Instrumenter(as) {
  }

  // Call instruction's address -> Snippet inserted here.
  // In self-propelled, we only have one snippet (or patch area) inserted per point
  typedef std::map<Address, SpSnippet::ptr> InstMap;
  InstMap g_inst_map;

  // We resort to trap to transfer control to patch area, when we are not able to
  // use jump-based implementation.
  void
  SpInstrumenter::trap_handler(int sig, siginfo_t* info, void* c) {
    Address pc = SpSnippet::get_pre_signal_pc(c) - 1;

    InstMap& inst_map = g_inst_map;
    if (inst_map.find(pc) == inst_map.end()) {
      return;
    }

    // Get patch area's address
    SpSnippet::ptr sp_snip = inst_map[pc];
    // Point* pt = sp_snip->point();

    char* blob = (char*)sp_snip->buf();
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    PatchMgrPtr mgr = g_context->mgr();
    SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(mgr->as());
    if (!as->set_range_perm((Address)blob, sp_snip->size(), perm)) {
      as->dump_mem_maps();
      exit(0);
    }

    // Set pc to jump to patch area
    SpSnippet::set_pc((Address)blob, c);
  }

  // The main routine for instrumentation.
  bool
  SpInstrumenter::run() {

    // Check each instrumented point, which is in a command
    for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
      PushBackCommand* command = static_cast<PushBackCommand*>(*c);

      if (command) {
        InstancePtr instance = command->instance();
        Point* pt = instance->point();
        SpPoint* spt = static_cast<SpPoint*>(pt);

        // If it is not a direct call, and we don't want to instrument direct call
        if (!pt->getCallee() && g_context->directcall_only()) continue;

#ifndef SP_RELEASE
        if (spt->instrumented()) {
          sp_debug("SKIPED - piont %lx is instrumented, skip it", pt->block()->last());
        }
#endif
        if (!spt->instrumented()) {
          Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
          SpSnippet::ptr sp_snip = snip->rep();
          spt->set_snip(sp_snip);
          Address eip = pt->block()->last();
          char* insn = (char*)eip;

          // Address insn_size = pt->block()->end() - eip;
          Address ret_addr = pt->block()->end();
          Instruction::Ptr callinsn = pt->block()->getInsn(eip);

          // If the call is made by a jump, which may be tail call optimization
          if (callinsn->getCategory() == in::c_BranchInsn) {
#ifndef SP_RELEASE
            sp_debug("TAIL CALL - point %lx is a tail call", pt->block()->last());
#endif
            spt->set_tailcall(true);
            ret_addr = 0;
          }

          char* blob = (char*)sp_snip->buf();
          bool realloc = false;

        REALLOC:
          long rel_addr = (long)blob - (long)eip;
          bool jump_abs = false;
          if (!sp::is_disp32(rel_addr)) {
#ifndef SP_RELEASE
            sp_debug("REL JUMP TOO BIG - can use relative jump");
#endif
						jump_abs = true;
					}

          // Save the original instruction, in case we want to restore it later.
          // TODO: should implement the undo() routine to undo everything
          // instrumented
          sp_snip->set_orig_call_insn(callinsn);

          // Indirect call
          if ((insn[0] != (char)0xe8) || jump_abs) {

            // First, we try to use jump to install instrumentation.
            if (install_indirect(spt, sp_snip, jump_abs, ret_addr)) {
              spt->set_instrumented(true);
            }
            // Reallocate the buffer
            else if (!realloc) {
              realloc = true;
              blob = (char*)sp_snip->realloc();
              if (blob) goto REALLOC;
            }

            // If jump fails, let's try trap ...
            if (realloc && !spt->instrumented()) {
              sp_print("FAILED to use JUMP - TRY TO USE TRAP");

              // Set trap handler for the worst case that spring jump doesn't work
              struct sigaction act;
              act.sa_sigaction = SpInstrumenter::trap_handler;
              act.sa_flags = SA_SIGINFO;
              struct sigaction old_act;
              sigaction(SIGTRAP, &act, &old_act);

              g_inst_map[eip] = sp_snip;
              blob = sp_snip->blob(ret_addr);

              if (install_trap(spt, blob, sp_snip->size())) {
                spt->set_install_method(SP_TRAP);
                spt->set_instrumented(true);
              } else {
                sp_print("FAILED to use TRAP, no instrumentation for this point");
              }
            }
          }

          // Direct call
          else {
            blob = sp_snip->blob(ret_addr);
            // Install the blob to pt
            if (install_direct(spt, blob, sp_snip->size())) {
              spt->set_install_method(SP_RELOC_INSN);
              spt->set_instrumented(true);
            } else {
              sp_print("FAILED - Failed to install instrumentation at %lx for calling %s",
                       pt->block()->last(), g_context->parser()->callee(pt)->name().c_str());
            }
          } // Direct call
        } // If not yet instrumented
      } // If it is a valid command
    } // For each command
    user_commands_.clear();

    return true;
  }

  bool
  SpInstrumenter::install_direct(SpPoint* point, char* blob, size_t blob_size) {

    char jump[5];
		memset(jump, 0, 5);
    char* p = jump;
    *p++ = 0xe9;
    int* lp = (int*)p;

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx for %s- {", point->block()->size(), point->block()->last(), point->callee()->name().c_str());
    sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->size()).c_str());
    sp_debug("}");

    // Build the jump instruction
    PatchObject* obj = point->block()->object();
    char* addr = (char*)point->block()->last();
    size_t insn_length = point->block()->end() - point->block()->last();
    *lp = (int)((long)blob - (long)addr - insn_length);
		sp_debug("REL CAL - blob at %lx, last insn at %lx, insn length %ld", (unsigned long)blob, point->block()->end(), insn_length);
		sp_debug("JUMP REL - jump to relative address %x", *lp);

    // Replace "call" with a "jump" instruction
    SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (as->set_range_perm((Address)addr, insn_length, perm)) {
      as->write(obj, (Address)addr, (Address)jump, 5);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Change the permission of snippet, so that it can be executed.
    if (!as->set_range_perm((Address)blob, blob_size, perm)) {
      sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", (Dyninst::Address)blob);
      as->dump_mem_maps();
      exit(0);
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, insn_length)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx for %s- {", point->block()->size(), point->block()->last(), point->callee()->name().c_str());
    sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->size()).c_str());
    sp_debug("}");

    return true;
  }

  bool
  SpInstrumenter::install_indirect(SpPoint* point, SpSnippet::ptr snip,
                                   bool jump_abs, Address ret_addr) {
    PatchBlock* blk = point->block();
    size_t blk_size = blk->size();
    string& orig_blk = snip->orig_blk();

    char* raw_blk = (char*)blk->start();
    for (unsigned i = 0; i < blk_size; i++) {
      orig_blk += raw_blk[i];
    }

    size_t limit = 0;
    // char* addr = NULL;
    // SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
    // int perm = PROT_READ | PROT_WRITE | PROT_EXEC;

    char insn[64];
    if (!jump_abs) {

      limit = 5; // one relative jump insn's size
      char* p = insn;
      *p++ = 0xe9;
      long* lp = (long*)p;
      *lp = (long)snip->buf() - (long)blk->start() - 5;
    } else {
      limit = snip->jump_abs_size();

      static_cast<SpPoint*>(point)->snip()->emit_jump_abs((long)snip->buf(), insn, 0, /*abs=*/true);
    }

    if (blk_size >= limit) {
			sp_debug("RELOC BLK - block size %lu bytes >= abs jump size %lu bytes", blk_size, limit);
      point->set_install_method(SP_RELOC_BLK);
      return install_jump(blk, insn, limit, snip, ret_addr);
    }

    point->set_install_method(SP_SPRINGBOARD);
		sp_debug("SPRING BLK - block size %lu bytes < abs jump size %lu bytes", blk_size, limit);
    return install_spring(blk, snip, ret_addr);
  }

  bool
  SpInstrumenter::install_jump(PatchBlock* blk,
                               char* insn, size_t insn_size,
                               SpSnippet::ptr snip,
                               Address ret_addr) {

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {", blk->size(), blk->last());
    sp_debug("%s", g_context->parser()->dump_insn((void*)blk->start(), blk->size()).c_str());
    sp_debug("}");

    // Build blob & change the permission of snippet
    char* blob = snip->blob(ret_addr, /*reloc=*/true);

    PatchObject* obj = blk->obj();
    SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (!as->set_range_perm((Address)blob, snip->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", (Dyninst::Address)blob);
      as->dump_mem_maps();
      exit(0);
    }

    char* addr = (char*)blk->start();

    // Write a "jump" instruction to call block
    if (as->set_range_perm((Address)addr, insn_size, perm)) {
      as->write(obj, (Address)addr, (Address)insn, insn_size);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, insn_size)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }
#ifndef SP_RELEASE
    sp_debug("USE BLK-RELOC - piont %lx is instrumented using call block relocation", blk->last());
#endif

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {", blk->size(), blk->last());
    sp_debug("%s", g_context->parser()->dump_insn((void*)blk->start(), blk->size()).c_str());
    sp_debug("}");

    return true;
  }

  // Install the patch area using a springboard
  bool
  SpInstrumenter::install_spring(PatchBlock* callblk,
                                 SpSnippet::ptr snip,
                                 Address ret_addr) {

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {", callblk->size(), callblk->last());
    sp_debug("%s", g_context->parser()->dump_insn((void*)callblk->start(), callblk->size()).c_str());
    sp_debug("}");

    // Find a spring block. A spring block should:
    // - big enough to hold two absolute jumps
    // - close enough to short jump from call block
    // if we cannot find one available spring block, just use trap or ignore
    //  this call
    PatchBlock* springblk = snip->spring_blk();
    if (!springblk) return false;

    // Build blob & change the permission of snippet
    char* blob = snip->blob(ret_addr, /*reloc=*/true, /*spring=*/true);
    PatchObject* obj = callblk->obj();
    SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (!as->set_range_perm((Address)blob, snip->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", (Dyninst::Address)blob);
      as->dump_mem_maps();
      exit(0);
    }

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {", callblk->size(), callblk->last());
    sp_debug("%s", g_context->parser()->dump_insn((void*)callblk->start(), callblk->size()).c_str());
    sp_debug("}");

    // Relocate spring block & change the permission of the relocated spring block
    char* spring = snip->spring(springblk->last());
    obj = springblk->obj();
    if (!as->set_range_perm((Address)spring, snip->spring_size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission for relocated spring blk at %lx", (Dyninst::Address)spring);
      as->dump_mem_maps();
      exit(0);
    }


    // Handle spring block
    // Write two "jump" instruction to spring block
    char springblk_insn[64];

    // First jump to relocated spring block
    size_t off = 0;
    size_t isize = snip->emit_jump_abs((long)spring, springblk_insn, off, /*abs=*/true);
    off += isize;
    size_t call_blk_jmp_trg = off;
    off = snip->emit_jump_abs((long)blob, springblk_insn, off, /*abs=*/true);
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
    callblk_insn[1] = (char)(springblk->start() + call_blk_jmp_trg - ((long)addr + 2));

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
    sp_debug("USE SPRING - piont %lx is instrumented using 1-hop spring",
             callblk->last());
#endif

    return true;
  }

  // Install the patch area, using trap.
  bool
  SpInstrumenter::install_trap(SpPoint* point, char* blob, size_t blob_size) {

    string int3;
    int3 += (char)0xcc;

    PatchObject* obj = point->block()->object();
    char* addr = (char*)point->block()->last();
    size_t insn_length = point->block()->end() - point->block()->last();
    for (unsigned i = 0; i < (insn_length-1); i++) {
      int3 += (char)0x90;
    }

    // Overwrite int3 to the call site
    SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(as_);
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    if (!as->set_range_perm((Address)addr, insn_length, perm)) {
      return false;
    } else {
      as->write(obj, (Address)addr, (Address)int3.c_str(), int3.size());
    }

    // Restore the permission of memory mapping
    if (!as->restore_range_perm((Address)addr, insn_length)) {
      return false;
    }
#ifndef SP_RELEASE
    sp_debug("USE TRAP - piont %lx is instrumented using trap", point->block()->last());
#endif
    return true;
  }
}
