#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

using Dyninst::PatchAPI::PatchFunction;

namespace sp {

SpSnippet::SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
                     Dyninst::PatchAPI::Point* pt,
                     SpContext* c,
                     PayloadFunc p)
  : func_(f), point_(pt), context_(c), payload_(p), blob_size_(0) {
  // FIXME: use AddrSpace::malloc later
  assert(context_ && "SpContext is NULL");
  Dyninst::PatchAPI::PatchMgrPtr mgr = c->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  blob_ = (char*)as->malloc(f->obj(), 1024, f->obj()->codeBase());
  /*
  blob_ = (char*)malloc(1024+ getpagesize() -1);
  blob_ = (char*)(((Dyninst::Address)blob_ + getpagesize()-1) & ~(getpagesize()-1));
  */
}

SpSnippet::~SpSnippet() {
  free(blob_);
}

/* For function call that is made by call insn, the blob contains:
   1. Save context
   2. Call payload function
   3. Restore context
   4. Call original function
   5. Jump back original address

   For function call that is made by jump insn, the blob contains:
   1. Save context
   2. Call payload function
   3. Restore context
   4. Jump to original function
 */
char* SpSnippet::blob(Dyninst::Address ret_addr) {
  assert(payload_);
  assert(context_);
  assert(func_);
  sp_debug("BLOB - patch area at %lx for calling %s, will return to %lx",
          blob_, func_->name().c_str(), ret_addr);
  if (blob_size_ > 0) {
    sp_debug("BLOB - Blob is constructed for calling %s(), just grab it!",
            func_->name().c_str());
    return blob_;
  }

  size_t offset = 0;
  size_t insnsize = 0;

  // 1. save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;

  // 2. pass parameters
  insnsize = emit_pass_param((long)point_, (long)context_, blob_, offset);
  offset += insnsize;

  // 3. call payload
  insnsize = emit_call_abs((long)payload_, blob_, offset);
  offset += insnsize;

  // 3. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  // 4. call ORIG_FUNCTION
  if (ret_addr == 0) {
    insnsize = emit_call_jump((long)func_->addr(), blob_, offset);
    offset += insnsize;
  } else {
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
    offset += insnsize;
    // 5. jmp ORIG_INSN_ADDR
    insnsize = emit_jump_abs(ret_addr, blob_, offset);
    offset += insnsize;
  }


  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");

  return blob_;
}

}
