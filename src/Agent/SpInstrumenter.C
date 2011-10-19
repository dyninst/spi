#include <sys/mman.h>

#include "Point.h"
#include "PatchCFG.h"
#include "SpInstrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"
#include "SpContext.h"
#include "SpAddrSpace.h"
#include "SpUtils.h"
#include "SpPoint.h"

using sp::SpContext;
using sp::SpSnippet;
using sp::JumpInstrumenter;

using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchBlock;

extern sp::SpContext* g_context;

/************************************************
  Jump-based instrumenter
 ************************************************/

JumpInstrumenter* JumpInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new JumpInstrumenter(as);
}


JumpInstrumenter::JumpInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

bool JumpInstrumenter::run() {

  // Check each instrumented point, which is in a command
  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand* command = static_cast<PushBackCommand*>(*c);

    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      sp::SpPoint* spt = static_cast<sp::SpPoint*>(pt);

      // If it is not a direct call, and we don't want to instrument direct call
      if (!pt->getCallee() && g_context->directcall_only()) continue;

      if (!spt->instrumented()) {
        Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
        SpSnippet::ptr sp_snip = snip->rep();
        Dyninst::Address eip = pt->block()->last();
        char* insn = (char*)eip;

        string& orig_insn = sp_snip->orig_insn();
        Dyninst::Address insn_size = pt->block()->end() - eip;
        Dyninst::Address ret_addr = pt->block()->end();
        Dyninst::InstructionAPI::Instruction::Ptr callinsn = pt->block()->getInsn(eip);

        // If the call is made by a jump, which may be tail call optimization
        if (callinsn->getCategory() == Dyninst::InstructionAPI::c_BranchInsn) {
          spt->set_tailcall(true);
          ret_addr = 0;
        }

        char* blob = (char*)sp_snip->buf();
        long abs_rel_addr = ((long)blob > (long)eip) ? ((long)blob - (long)eip) : ((long)eip - (long)blob);
        // Save the original instruction, in case we want to restore it later
        for (int i = 0; i < insn_size; i++) {
          orig_insn += insn[i];
        }

        //---------------------------------------------------------
        // Indirect call or call insn will be bigger than 5 bytes
        //---------------------------------------------------------
        if ((insn_size < 5) || (abs_rel_addr > 0xffffffff)) {
          bool jump_abs = false;
          if (abs_rel_addr > 0xffffffff) jump_abs = true;
          if (install_indirect(pt, sp_snip, jump_abs, ret_addr)) {
            spt->set_instrumented(true);
          } else {
            sp_print("FAILED");
            //sp_print("FAILED - Failed to install instrumentation at %lx for calling %s",
            //         pt->block()->last(), g_context->parser()->callee(pt)->name().c_str());
          }
        }

        //---------------------------------------------------------
        // Direct call
        //---------------------------------------------------------
        else {
          blob = sp_snip->blob(ret_addr);

          // Install the blob to pt
          if (install_direct(pt, blob, sp_snip->size())) {
            spt->set_instrumented(true);
          } else {
            sp_print("FAILED - Failed to install instrumentation at %lx for calling %s",
                     pt->block()->last(), g_context->parser()->callee(pt)->name().c_str());
          }
        } // direct call

      } // If not yet instrumented
    } // If it is a valid command
  } // For each command
  user_commands_.clear();

  return true;
}

bool JumpInstrumenter::install_direct(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size) {

  char jump[5];
  char* p = jump;
  *p++ = 0xe9;
  long* lp = (long*)p;
  /*
  sp_print("before install");
  sp_print("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end()-point->block()->start()).c_str());
  sp_print("DUMP INSN - }");
  */
  // Build the jump instruction
  Dyninst::PatchAPI::PatchObject* obj = point->block()->object();
  char* addr = (char*)point->block()->last();
  size_t insn_length = point->block()->end() - point->block()->last();
  *lp = (long)blob - (long)addr - insn_length;

  // Replace "call" with a "jump" instruction
  SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (as->set_range_perm((Dyninst::Address)addr, insn_length, perm)) {
    as->write(obj, (Dyninst::Address)addr, (Dyninst::Address)jump, 5);
  } else {
    sp_print("MPROTECT - Failed to change memory access permission");
  }

  // Change the permission of snippet, so that it can be executed.
  if (!as->set_range_perm((Dyninst::Address)blob, blob_size, perm)) {
    sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", blob);
    as->dump_mem_maps();
    exit(0);
  }

  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_length)) {
    sp_print("MPROTECT - Failed to restore memory access permission");
  }
  /*
  sp_print("after install");
  sp_print("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end()-point->block()->start()).c_str());
  sp_print("DUMP INSN - }");
*/
  return true;
}

bool JumpInstrumenter::install_indirect(Dyninst::PatchAPI::Point* point,
                                        sp::SpSnippet::ptr snip, bool jump_abs,
                                        Dyninst::Address ret_addr) {
  PatchBlock* blk = point->block();
  size_t blk_size = blk->size();
  string& orig_blk = snip->orig_blk();


  char* raw_blk = (char*)blk->start();
  for (int i = 0; i < blk_size; i++) {
    orig_blk += raw_blk[i];
  }

  size_t limit = 0;
  char* addr = NULL;

  SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;

  char insn[64];
  if (!jump_abs) {
    sp_debug("JUMP REL");

    limit = 5; // one jump_rel insn
    char* p = insn;
    *p++ = 0xe9;
    long* lp = (long*)p;
    *lp = (long)snip->buf() - (long)blk->start() - 5;
  } else {
    sp_debug("JUMP ABS");
    limit = snip->jump_abs_size();

    //XXX: skip for now
    return true;
  }
  /*
  if (blk_size >= limit) {
    sp_debug("RELOC BLK - jump");
    return install_jump(blk, insn, limit, snip, ret_addr);
  }
*/
  return install_spring(blk, snip, ret_addr);
}

bool JumpInstrumenter::install_jump(Dyninst::PatchAPI::PatchBlock* blk,
                                    char* insn, size_t insn_size,
                                    sp::SpSnippet::ptr snip,
                                    Dyninst::Address ret_addr) {

  sp_debug("before install");
  sp_debug("%s", g_context->parser()->dump_insn((void*)blk->start(), blk->end()-blk->start()).c_str());
  sp_debug("DUMP INSN - }");

  // Build blob & change the permission of snippet
  char* blob = snip->blob(ret_addr, /*reloc=*/true);
  Dyninst::PatchAPI::PatchObject* obj = blk->obj();
  SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)blob, snip->size(), perm)) {
    sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", blob);
    as->dump_mem_maps();
    exit(0);
  }

  char* addr = (char*)blk->start();
  // Write a "jump" instruction to call block
  if (as->set_range_perm((Dyninst::Address)addr, insn_size, perm)) {
    as->write(obj, (Dyninst::Address)addr, (Dyninst::Address)insn, insn_size);
  } else {
    sp_print("MPROTECT - Failed to change memory access permission");
  }


  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_size)) {
    sp_print("MPROTECT - Failed to restore memory access permission");
  }

  sp_debug("after install");
  sp_debug("%s", g_context->parser()->dump_insn((void*)blk->start(), blk->end()-blk->start()).c_str());
  sp_debug("DUMP INSN - }");

  return true;
}

bool JumpInstrumenter::install_spring(Dyninst::PatchAPI::PatchBlock* callblk,
                                      sp::SpSnippet::ptr snip,
                                      Dyninst::Address ret_addr) {


  // Find a spring block. A spring block should:
  // - big enough to hold two absolute jumps
  // - close enough to short jump from call block
  // if we cannot find one available spring block, just use trap or ignore this call
  PatchBlock* springblk = snip->spring_blk();
  if (!springblk) return false;

  // Build blob & change the permission of snippet
  char* blob = snip->blob(ret_addr, /*reloc=*/true, /*spring=*/true);
  Dyninst::PatchAPI::PatchObject* obj = callblk->obj();
  SpAddrSpace* as = static_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)blob, snip->size(), perm)) {
    sp_print("MPROTECT - Failed to change memory access permission for blob at %lx", blob);
    as->dump_mem_maps();
    exit(0);
  }

  // Relocate spring block & change the permission of the relocated spring block
  char* spring = snip->spring(springblk->last());
  obj = springblk->obj();
  if (!as->set_range_perm((Dyninst::Address)spring, snip->spring_size(), perm)) {
    sp_print("MPROTECT - Failed to change memory access permission for relocated spring blk at %lx", spring);
    as->dump_mem_maps();
    exit(0);
  }


  //------------------------------------------------------
  // Handle spring block
  //------------------------------------------------------
  sp_debug("before install (spring block)");
  sp_debug("%s", g_context->parser()->dump_insn((void*)springblk->start(),
           springblk->end()-springblk->start()).c_str());
  sp_debug("DUMP INSN - }");

  // Write two "jump" instruction to spring block
  char springblk_insn[64];
  // first jump to relocated spring block
  size_t off = 0;
  size_t isize = SpSnippet::emit_jump_abs((long)spring, springblk_insn, off, /*abs=*/true);
  off += isize;
  size_t call_blk_jmp_trg = off;
  off = SpSnippet::emit_jump_abs((long)blob, springblk_insn, off, /*abs=*/true);
  off += isize;
  // second jump to relocated spring block

  char* addr = (char*)springblk->start();
  if (as->set_range_perm((Dyninst::Address)addr, springblk->size(), perm)) {
    as->write(obj, (Dyninst::Address)addr, (Dyninst::Address)springblk_insn, off);
  } else {
    sp_print("MPROTECT - Failed to change memory access permission");
  }
  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, springblk->size())) {
    sp_print("MPROTECT - Failed to restore memory access permission");
  }
  sp_debug("after install (spring block)");
  sp_debug("%s", g_context->parser()->dump_insn((void*)springblk->start(), springblk->end()-springblk->start()).c_str());
  sp_debug("DUMP INSN - }");


  //------------------------------------------------------
  // Handle call block
  //------------------------------------------------------
  // Write a "jump" instruction to call block
  sp_debug("before install (call block)");
  sp_debug("%s", g_context->parser()->dump_insn((void*)callblk->start(), callblk->end()- callblk->start()).c_str());
  sp_debug("DUMP INSN - }");

  addr = (char*)callblk->start();
  char callblk_insn[2];
  callblk_insn[0] = 0xeb;
  callblk_insn[1] = (char)(springblk->start() + call_blk_jmp_trg - ((long)addr + 2));

  if (as->set_range_perm((Dyninst::Address)addr, callblk->size(), perm)) {
    as->write(obj, (Dyninst::Address)addr, (Dyninst::Address)callblk_insn, 2);
  } else {
    sp_print("MPROTECT - Failed to change memory access permission");
  }
  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, callblk->size())) {
    sp_print("MPROTECT - Failed to restore memory access permission");
  }
  sp_debug("after install (call block)");
  sp_debug("%s", g_context->parser()->dump_insn((void*)callblk->start(), callblk->end()-callblk->start()).c_str());
  sp_debug("DUMP INSN - }");


  return true;
}
