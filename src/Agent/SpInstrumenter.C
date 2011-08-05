#include <sys/mman.h>

#include "Point.h"
#include "PatchCFG.h"
#include "SpInstrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"
#include "SpContext.h"

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

bool SpInstrumenter::run() {
  sp_debug("CODE GEN - Start instrumentation and generate binary, %d commands to go", user_commands_.size());

  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand::Ptr command = DYN_CAST(PushBackCommand, *c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();

      // 1. Generate code for snippet
      char* blob = sp_snip->blob();

      // 2. Link snippet to original code
      if (install(pt, blob, sp_snip->context())) {
        sp_debug("INSTALLED - Instrumentation at %lx for calling %s",
                 pt->getBlock()->last(), pt->getCallee()->name().c_str());
      } else {
        sp_debug("FAILED - Failed to install instrumentation at %lx for calling %s",
                 pt->getBlock()->last(), pt->getCallee()->name().c_str());
      }
    }
  }
}

bool SpInstrumenter::install(Point* point, char* blob, SpContextPtr context) {
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

  sp_debug("BEGIN DUMP INSN {\n\n%s", context->parser()->dump_insn((void*)point->getBlock()->start(),
                                              point->getBlock()->size()).c_str());
  sp_debug("} END DUMP INSN");
  if (mprotect(obj_base, obj->co()->cs()->length(), PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  } else {
    // memcpy(addr, int3, sizeof(int3));
    sp_debug("WRITE - jmp %lx", rel_dist);
  }
  if (mprotect(obj_base, obj->co()->cs()->length(), PROT_EXEC) < 0) {
    sp_debug("MPROTECT - Failed to change memory access permission");
  }
  return true;
}
