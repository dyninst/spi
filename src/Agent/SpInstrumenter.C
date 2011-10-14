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
using sp::TrapInstrumenter;
using sp::JumpInstrumenter;

using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchMgrPtr;

extern sp::SpContext* g_context;

/**************************************************
   Trap-based instrumenter
 **************************************************/
TrapInstrumenter* TrapInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new TrapInstrumenter(as);
}

TrapInstrumenter::TrapInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

void trap_handler(int sig, siginfo_t* info, void* c) {

  // get pc
  Dyninst::Address pc = sp::SpSnippet::get_pre_signal_pc(c) - 1;
  SpContext::InstMap inst_map = g_context->inst_map();
  if (inst_map.find(pc) == inst_map.end()) {
    return;
  }

  // get patch area's address
  InstancePtr instance = inst_map[pc];
  Point* pt = instance->point();
  Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
  SpSnippet::ptr sp_snip = snip->rep();

  Dyninst::Address ret_addr = pc+1;
  Dyninst::PatchAPI::PatchBlock* blk = pt->block();
  Dyninst::Address last = blk->last();
  Dyninst::InstructionAPI::Instruction::Ptr callinsn = blk->getInsn(last);
  if (callinsn->getCategory() == Dyninst::InstructionAPI::c_BranchInsn) {
    ret_addr = 0;
  }

  char* blob = sp_snip->blob(ret_addr);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  PatchMgrPtr mgr = g_context->mgr();
  sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
  if (!as->set_range_perm((Dyninst::Address)blob, sp_snip->size(), perm)) {
    as->dump_mem_maps();
    exit(0);
  }

  sp_snip->dump_context((ucontext_t*)c);
  g_context->parser()->set_old_context((ucontext_t*)c);
  sp::SpSnippet::set_pc((Dyninst::Address)blob, c);
}

bool TrapInstrumenter::run() {
  sp::instrumenter_start();

  // Use trap to do instrumentation
  struct sigaction act;
  act.sa_sigaction = trap_handler;
  act.sa_flags = SA_SIGINFO;
  struct sigaction old_act;
  sigaction(SIGTRAP, &act, &old_act);

  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand* command = dynamic_cast<PushBackCommand*>(*c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      if (!pt->getCallee() && g_context->directcall_only()) continue;

      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();

      // 1. Logically link snippet to the point (build map)
      Dyninst::Address eip = pt->block()->last();
      SpContext::InstMap& inst_map = g_context->inst_map();

      // 2. If this point is already instrumented, skip it
      if (inst_map.find(eip) == inst_map.end()) {
        inst_map[eip] = instance;
        int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
        PatchMgrPtr mgr = g_context->mgr();
        sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());

        string& orig_insn = sp_snip->orig_insn();
        Dyninst::Address insn_size = pt->block()->end() - eip;

        // Change permission of the original call instructin, so that we can
        // write an int3 to it
        if (!as->set_range_perm((Dyninst::Address)eip, insn_size, perm)) {
          sp_perror("MPROTECT - Failed to change memory access permission");
        };
        char* insn = (char*)eip;
        for (int i = 0; i < insn_size; i++) {
          orig_insn += insn[i];
        }

        // Install the blob to pt
        if (install(pt, NULL, sp_snip->size())) {

        }
      }
    }
  }
  user_commands_.clear();
  // Things to be restored
  g_context->set_old_act(old_act);
  sp::instrumenter_end();
}

bool TrapInstrumenter::install(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size) {
  string int3;
  int3 += (char)0xcc;

  Dyninst::PatchAPI::PatchObject* obj = point->block()->object();
  char* addr = (char*)point->block()->last();
  size_t insn_length = point->block()->end() - point->block()->last();
  for (int i = 0; i < (insn_length-1); i++) {
    int3 += (char)0x90;
  }
  // Write int3 to the call site
  SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)addr, insn_length, perm)) {
  } else {
    as->write(obj, (Dyninst::Address)addr, (Dyninst::Address)int3.c_str(), int3.size());
  }

  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_length)) {
  }

  sp::install_end();
  return true;
}

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

        char* blob = sp_snip->blob(ret_addr);
        long abs_rel_addr = ((long)blob > (long)eip) ? ((long)blob - (long)eip) : ((long)eip - (long)blob);

        // FIXME: for now, we only instrument:
        //        - original call insn_size >= 5 and
        //        - relative addr to snippet <= 4
        if ((insn_size < 5) || (abs_rel_addr > 0xffffffff)) {
          continue;
        }

        // Save the original instruction, in case we want to restore it later
        for (int i = 0; i < insn_size; i++) {
          orig_insn += insn[i];
        }

        // Install the blob to pt
        if (install(pt, blob, sp_snip->size())) {
          spt->set_instrumented(true);
        } else {
          sp_print("FAILED - Failed to install instrumentation at %lx for calling %s",
                   pt->block()->last(), g_context->parser()->callee(pt)->name().c_str());
        }
      } // If not yet instrumented
    } // If it is a valid command
  } // For each command
  user_commands_.clear();

  return true;
}

bool JumpInstrumenter::install(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size) {

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
