#include <sys/mman.h>

#include "Point.h"
#include "PatchCFG.h"
#include "SpInstrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"
#include "SpContext.h"
#include "SpAddrSpace.h"

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
namespace sp {
extern Dyninst::Address get_pre_signal_pc(void* context);
extern Dyninst::Address set_pc(Dyninst::Address pc, void* context);
extern void dump_context(ucontext_t* context);
}

TrapInstrumenter* TrapInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new TrapInstrumenter(as);
}


TrapInstrumenter::TrapInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

void simple_trap_handler(int sig, siginfo_t* info, void* c) {

  // Get pc
  Dyninst::Address pc = sp::get_pre_signal_pc(c) - 1;
  sp_debug("TRAP - Executing payload code for address %lx", pc);
  SpContext::InstMap inst_map = g_context->inst_map();
  if (inst_map.find(pc) == inst_map.end()) return;
  InstancePtr instance = inst_map[pc];
  Point* pt = instance->point();
  Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
  SpSnippet::ptr sp_snip = snip->rep();

  // Execute payload function
  sp_debug("PAYLOAD - At address %lx", sp_snip->payload());
  sp::PayloadFunc_t payload_func = (sp::PayloadFunc_t)sp_snip->payload();
  payload_func(pt, g_context);

  // Restore the original instruction
  string& orig_insn = sp_snip->orig_insn();
  PatchMgrPtr mgr = g_context->mgr();
  sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)pc, orig_insn.size(), perm)) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    memcpy((void*)pc, orig_insn.c_str(), orig_insn.size());
  }
  if (!as->restore_range_perm((Dyninst::Address)pc, orig_insn.size())) {
    sp_debug("MPROTECT - Failed to restore memory access permission");
  }
  sp::set_pc(pc, c);
}

void trap_handler(int sig, siginfo_t* info, void* c) {
  // get pc
  Dyninst::Address pc = sp::get_pre_signal_pc(c) - 1;
  sp_debug("TRAP - Executing payload code for address %lx", pc);
  SpContext::InstMap inst_map = g_context->inst_map();
  if (inst_map.find(pc) == inst_map.end()) return;

  // get patch area's address
  InstancePtr instance = inst_map[pc];
  Point* pt = instance->point();
  Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
  SpSnippet::ptr sp_snip = snip->rep();
  sp_debug("IN TRAP - old_context: %lx", c);
  sp_snip->set_old_context((ucontext_t*)c);

  char* blob = sp_snip->blob(pc+1 /*+1 is for after int3*/);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  PatchMgrPtr mgr = g_context->mgr();
  sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
  if (!as->set_range_perm((Dyninst::Address)blob, sp_snip->size(), perm)) {
    sp_debug("MPROTECT - Failed to change memory access permission for blob at %lx", blob);
    as->dump_mem_maps();
    exit(0);
  }

  sp::dump_context((ucontext_t*)c);
  // set pc to patch area
  sp::set_pc((Dyninst::Address)blob, c);
}

bool TrapInstrumenter::run() {
  sp_debug("CODE GEN - Start trap_instrumentation and generate binary, %d commands to go", user_commands_.size());

  // Use trap to do instrumentation
  struct sigaction act;
  act.sa_sigaction = trap_handler;
  //act.sa_sigaction = simple_trap_handler;
  act.sa_flags = SA_SIGINFO;
  struct sigaction old_act;
  sigaction(SIGTRAP, &act, &old_act);

  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand* command = dynamic_cast<PushBackCommand*>(*c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();
      // 1. Logically link snippet to the point (build map)
      Dyninst::Address eip = pt->block()->last();
      SpContext::InstMap& inst_map = g_context->inst_map();

      // 2. If this point is already instrumented, skip it
      if (inst_map.find(eip) == inst_map.end()) {
        inst_map[eip] = instance;
        // char* blob = sp_snip->blob(eip+1 /*+1 is for int3*/);
        int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
        PatchMgrPtr mgr = g_context->mgr();
        sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
        //if (!as->set_range_perm((Dyninst::Address)blob, sp_snip->size(), perm)) {
        //  sp_debug("MPROTECT - Failed to change memory access permission for blob");
        //}

        string& orig_insn = sp_snip->orig_insn();
        Dyninst::Address insn_size = pt->block()->end() - eip;

        // Change permission of the original call instructin, so that we can
        // write an int3 to it
        if (!as->set_range_perm((Dyninst::Address)eip, insn_size, perm)) {
          sp_perror("MPROTECT - Failed to change memory access permission");
        };
        char* insn = (char*)eip;
        char buf[255];
        for (int i = 0; i < insn_size; i++) {
          orig_insn += insn[i];
        }
        sp_debug("ORIG INSN - %s", g_context->parser()->dump_insn((void*)insn, insn_size).c_str());

        // Install the blob to pt
        if (install(pt, NULL, sp_snip->size())) {
          sp_debug("INSTALLED - Instrumentation at %lx for calling %s",
                   pt->block()->last(), pt->getCallee()->name().c_str());
        } else {
          sp_debug("FAILED - Failed to install instrumentation at %lx for calling %s",
                   pt->block()->last(), pt->getCallee()->name().c_str());
        }
      }
    }
  }
  user_commands_.clear();
  // Things to be restored
  g_context->set_old_act(old_act);

}

bool TrapInstrumenter::install(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size) {
  // sp_debug("INSTALLING - blob %d bytes {", blob_size);
  // sp_debug("%s", g_context->parser()->dump_insn((void*)blob, blob_size).c_str());
  // sp_debug("}");

  string int3;
  int3 += (char)0xcc;

  sp_debug("CALL BLOCK - blob %d bytes {", blob_size);
  sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end() - point->block()->start()).c_str());
  sp_debug("}");

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
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    memcpy(addr, int3.c_str(), int3.size());
  }

  sp_debug("CALL BLOCK after - blob %d bytes {", blob_size);
  sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end() - point->block()->start()).c_str());
  sp_debug("}");

  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_length)) {
    sp_debug("MPROTECT - Failed to restore memory access permission");
  }

  return true;
}

/*
  Jump-based instrumenter
*/

JumpInstrumenter* JumpInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new JumpInstrumenter(as);
}


JumpInstrumenter::JumpInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

bool JumpInstrumenter::run() {
  sp_debug("CODE GEN - Start trap_instrumentation and generate binary, %d commands to go", user_commands_.size());
  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand* command = dynamic_cast<PushBackCommand*>(*c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();
      // 1. Logically link snippet to the point (build map)
      Dyninst::Address eip = pt->block()->last();
      SpContext::InstMap& inst_map = g_context->inst_map();
      // 2. If this point is already instrumented, skip it
      if (inst_map.find(eip) == inst_map.end()) {
        PatchMgrPtr mgr = g_context->mgr();
        sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());

        string& orig_insn = sp_snip->orig_insn();
        Dyninst::Address insn_size = pt->block()->end() - eip;
        Dyninst::Address ret_addr = pt->block()->end();//eip + 5;
        char* blob = sp_snip->blob(ret_addr);
        long abs_rel_addr = ((long)blob > (long)eip) ? ((long)blob - (long)eip) : ((long)eip - (long)blob);

        // FIXME: for now, we only instrument:
        //        - original call insn_size >= 5 and
        //        - relative addr to snippet <= 4
        if ((insn_size < 5) || (abs_rel_addr > 0xffffffff)) {
            sp_debug("CANNOT HANDLE - original call insn size (%d) < 5 or relative addr (%x - %x) to snippet > 2^32",
                     insn_size, (Dyninst::Address)blob, eip);
          continue;
        }
        sp_debug("CAN HANDLE - original call insn size (%d) < 5 or relative addr (%x - %x) to snippet > 2^32",
                 insn_size, (Dyninst::Address)blob, eip);

        inst_map[eip] = instance;

        char* insn = (char*)eip;
        char buf[255];
        for (int i = 0; i < insn_size; i++) {
          orig_insn += insn[i];
        }
        sp_debug("ORIG INSN - %s", g_context->parser()->dump_insn((void*)insn, insn_size).c_str());

        // Install the blob to pt
        if (install(pt, blob, sp_snip->size())) {
          sp_debug("INSTALLED - Instrumentation at %lx for calling %s",
                   pt->block()->last(), pt->getCallee()->name().c_str());
        } else {
          sp_debug("FAILED - Failed to install instrumentation at %lx for calling %s",
                   pt->block()->last(), pt->getCallee()->name().c_str());
        }
      }
    }
  }
  user_commands_.clear();

  return true;
}

bool JumpInstrumenter::install(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size) {
  char jump[5];
  char* p = jump;
  *p++ = 0xe9;
  long* lp = (long*)p;

  sp_debug("CALL BLOCK - blob %d bytes {", blob_size);
  sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end() - point->block()->start()).c_str());
  sp_debug("}");

  Dyninst::PatchAPI::PatchObject* obj = point->block()->object();
  char* addr = (char*)point->block()->last();
  size_t insn_length = point->block()->end() - point->block()->last();
  *lp = (long)blob - (long)addr - insn_length;

  SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)addr, insn_length, perm)) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    memcpy(addr, jump, 5);
  }
  if (!as->set_range_perm((Dyninst::Address)blob, blob_size, perm)) {
    sp_debug("MPROTECT - Failed to change memory access permission for blob at %lx", blob);
    as->dump_mem_maps();
    exit(0);
  }

  sp_debug("CALL BLOCK after - blob %d bytes {", blob_size);
  sp_debug("%s", g_context->parser()->dump_insn((void*)point->block()->start(), point->block()->end() - point->block()->start()).c_str());
  sp_debug("}");

  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_length)) {
    sp_debug("MPROTECT - Failed to restore memory access permission");
  }

  return true;
}
