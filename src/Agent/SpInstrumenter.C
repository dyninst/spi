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
using sp::SpInstrumenter;
using sp::TrapInstrumenter;

using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchMgrPtr;


SpInstrumenter* SpInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new SpInstrumenter(as);
}


SpInstrumenter::SpInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

extern sp::SpContextPtr g_context;
namespace sp {
extern Dyninst::Address get_pre_signal_pc(void* context);
extern Dyninst::Address set_pc(Dyninst::Address pc, void* context);
}

void default_trap_handler(int sig, siginfo_t* info, void* c) {
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

bool SpInstrumenter::run() {
  sp_debug("CODE GEN - Start instrumentation and generate binary, %d commands to go", user_commands_.size());
  // Use trap to do instrumentation
  struct sigaction act;
  act.sa_sigaction = default_trap_handler;
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
      char* blob = sp_snip->blob();
      Dyninst::Address eip = pt->block()->last();
      SpContext::InstMap& inst_map = g_context->inst_map();
      sp_debug("after inst_map");

      inst_map[eip] = instance;
      string& orig_insn = sp_snip->orig_insn();
      Dyninst::Address insn_size = pt->block()->end() - eip;
      sp_debug("after insn_size, end: %lx, last: %lx", pt->block()->end(), eip);

      PatchMgrPtr mgr = g_context->mgr();
      sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
      int perm = PROT_READ | PROT_WRITE | PROT_EXEC;

      if (!as->set_range_perm((Dyninst::Address)eip, insn_size, perm)) {
        sp_perror("MPROTECT - Failed to change memory access permission");
      };

      char* insn = (char*)eip;
      sp_debug("insn: %lx, size: %d", insn, insn_size);
      char buf[255];
      for (int i = 0; i < insn_size; i++) {
        orig_insn += insn[i];
      }
      sp_debug("after orig_insn");
      // 2. Physically link snippet to the point (generate code)
      if (install(pt, blob)) {
        sp_debug("INSTALLED - Instrumentation at %lx for calling %s",
                 pt->block()->last(), pt->getCallee()->name().c_str());
      } else {
        sp_debug("FAILED - Failed to install instrumentation at %lx for calling %s",
                 pt->block()->last(), pt->getCallee()->name().c_str());
      }
    }
  }
  user_commands_.clear();
  // Things to be restored
  g_context->set_old_act(old_act);
}


bool SpInstrumenter::install(Point* point, char* blob) {

  string int3;
  int3 += (char)0xcc;

  Dyninst::PatchAPI::PatchObject* obj = point->block()->object();
  char* addr = (char*)point->block()->last();
  size_t insn_length = point->block()->end() - point->block()->last();

  sp_debug("BEFORE INSTALL");
  sp_debug("BEGIN DUMP INSN {\n\n%s", g_context->parser()->dump_insn((void*)point->block()->start(),
                                              point->block()->size()).c_str());
  sp_debug("} END DUMP INSN");

  // Write int3 to the call site
  SpAddrSpace* as = dynamic_cast<SpAddrSpace*>(as_);
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  if (!as->set_range_perm((Dyninst::Address)addr, insn_length, perm)) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    memcpy(addr, int3.c_str(), int3.size());
  }

  // Restore the permission of memory mapping
  if (!as->restore_range_perm((Dyninst::Address)addr, insn_length)) {
    sp_debug("MPROTECT - Failed to restore memory access permission");
  }

  sp_debug("AFTER INSTALL");
  sp_debug("BEGIN DUMP INSN {\n\n%s", g_context->parser()->dump_insn((void*)point->block()->start(),
                                              point->block()->size()).c_str());
  sp_debug("} END DUMP INSN");

  return true;
}

TrapInstrumenter* TrapInstrumenter::create(Dyninst::PatchAPI::AddrSpace* as) {
  return new TrapInstrumenter(as);
}


TrapInstrumenter::TrapInstrumenter(Dyninst::PatchAPI::AddrSpace* as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

bool TrapInstrumenter::run() {
  sp_debug("CODE GEN - Start trap_instrumentation and generate binary, %d commands to go", user_commands_.size());

  // Use trap to do instrumentation
  struct sigaction act;
  act.sa_sigaction = default_trap_handler;
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
      char* blob = sp_snip->blob();
      Dyninst::Address eip = pt->block()->last();
      SpContext::InstMap& inst_map = g_context->inst_map();
      sp_debug("after inst_map");

      // 2. If this point is already instrumented, skip it
      if (inst_map.find(eip) == inst_map.end()) {
        inst_map[eip] = instance;
        string& orig_insn = sp_snip->orig_insn();
        Dyninst::Address insn_size = pt->block()->end() - eip;

        // Change permission of the original call instructin, so that we can
        // write an int3 to it
        PatchMgrPtr mgr = g_context->mgr();
        sp::SpAddrSpace* as = dynamic_cast<sp::SpAddrSpace*>(mgr->as());
        int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
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
        if (install(pt, blob)) {
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

bool TrapInstrumenter::install(Dyninst::PatchAPI::Point* point, char* blob) {
  sp_debug("INSTALLING - blob");
  return true;
}
