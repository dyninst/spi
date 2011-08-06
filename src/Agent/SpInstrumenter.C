#include <sys/mman.h>

#include "Point.h"
#include "PatchCFG.h"
#include "SpInstrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"
#include "SpContext.h"

using sp::SpContext;
using sp::SpSnippet;
using sp::SpInstrumenter;
using Dyninst::PatchAPI::CommandList;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Snippet;

SpInstrumenter::ptr SpInstrumenter::create(Dyninst::PatchAPI::AddrSpacePtr as) {
  return ptr(new SpInstrumenter(as));
}


SpInstrumenter::SpInstrumenter(Dyninst::PatchAPI::AddrSpacePtr as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

extern sp::SpContextPtr g_context;
namespace sp {
extern Dyninst::Address get_pre_signal_pc(void* context);
extern Dyninst::Address set_pc(Dyninst::Address pc, void* context);
}

void trap_handler(int sig, siginfo_t* info, void* c) {
  // Get eip
  Dyninst::Address pc = sp::get_pre_signal_pc(c) - 1;
  sp_debug("TRAP - Executing payload code for address %lx", pc);
  SpContext::InstMap inst_map = g_context->inst_map();
  if (inst_map.find(pc) == inst_map.end()) return;
  SpSnippet::ptr sp_snip = inst_map[pc];
  sp_debug("PAYLOAD - At address %lx", sp_snip->payload());
  sp::PayloadFunc_t payload_func = (sp::PayloadFunc_t)sp_snip->payload();
  payload_func(sp_snip->func(), g_context);
  string& orig_insn = sp_snip->orig_insn();
  memcpy((void*)pc, orig_insn.c_str(), orig_insn.size());
  sp::set_pc(pc, c);
}

bool SpInstrumenter::run() {
  sp_debug("CODE GEN - Start instrumentation and generate binary, %d commands to go", user_commands_.size());
  // Use trap to do instrumentation
  struct sigaction act;
  act.sa_sigaction = trap_handler;
  act.sa_flags = SA_SIGINFO;
  struct sigaction old_act;
  sigaction(SIGTRAP, &act, &old_act);

  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand::Ptr command = DYN_CAST(PushBackCommand, *c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();

      // 1. Logically link snippet to the point (build map)
      char* blob = sp_snip->blob();
      Dyninst::Address eip = pt->getBlock()->last();
      sp_debug("END OF BLOCK - %lx", eip);
      SpContext::InstMap& inst_map = g_context->inst_map();
      inst_map[eip] = sp_snip;
      string& orig_insn = sp_snip->orig_insn();
      Dyninst::Address insn_size = pt->getBlock()->end() - eip;
      char* insn = (char*)eip;
      for (int i = 0; i < insn_size; i++)
        orig_insn += insn[i];

      // 2. Physically link snippet to the point (generate code)
      if (install(pt, blob)) {
        sp_debug("INSTALLED - Instrumentation at %lx for calling %s",
                 pt->getBlock()->last(), pt->getCallee()->name().c_str());
      } else {
        sp_debug("FAILED - Failed to install instrumentation at %lx for calling %s",
                 pt->getBlock()->last(), pt->getCallee()->name().c_str());
      }
    }
  }
  user_commands_.clear();
  // Things to be restored
  g_context->set_old_act(old_act);
}


bool SpInstrumenter::install(Point* point, char* blob) {

  char int3[] = { 0xcc, 0x90, 0x90, 0x90, 0x90 };
  Dyninst::PatchAPI::PatchObject* obj = point->getBlock()->function()->object();
  char* addr = (char*)point->getBlock()->last();
  char* obj_base = (char*)obj->codeBase();
  size_t page_size = getpagesize();
  size_t rel_dist = (long)blob > (long)addr ?
    ((long)blob - (long)addr) :
    ((long)addr - (long)blob);
  // mprotect requires the address be aligned by page size
  while ((size_t)obj_base % page_size != 0) obj_base++;
  sp_debug("BEFORE INSTALL");
  sp_debug("BEGIN DUMP INSN {\n\n%s", g_context->parser()->dump_insn((void*)point->getBlock()->start(),
                                              point->getBlock()->size()).c_str());
  sp_debug("} END DUMP INSN");
  if (mprotect(obj_base, obj->co()->cs()->length(), PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    memcpy(addr, int3, sizeof(int3));
    sp_debug("WRITE - jmp %lx", rel_dist);
  }
  // if (mprotect(obj_base, obj->co()->cs()->length(), PROT_EXEC) < 0) {
  //  sp_debug("MPROTECT - Failed to change memory access permission");
  //}
  sp_debug("AFTER INSTALL");
  sp_debug("BEGIN DUMP INSN {\n\n%s", g_context->parser()->dump_insn((void*)point->getBlock()->start(),
                                              point->getBlock()->size()).c_str());
  sp_debug("} END DUMP INSN");

  return true;
}
