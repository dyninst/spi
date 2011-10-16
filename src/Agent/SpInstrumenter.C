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
