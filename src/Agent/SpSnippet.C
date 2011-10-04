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
                     PayloadFunc head, PayloadFunc tail)
  : func_(f), point_(pt), context_(c), head_(head), tail_(tail), blob_size_(0) {
  // FIXME: use AddrSpace::malloc later
  assert(context_ && "SpContext is NULL");
  Dyninst::PatchAPI::PatchMgrPtr mgr = c->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  blob_ = (char*)as->malloc(pt->obj(), 1024, pt->obj()->codeBase());
  /*
  blob_ = (char*)malloc(1024+ getpagesize() -1);
  blob_ = (char*)(((Dyninst::Address)blob_ + getpagesize()-1) & ~(getpagesize()-1));
  */
}

SpSnippet::~SpSnippet() {
  free(blob_);
}

size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset) {
  char* p = buf + offset;
  char* psrc = (char*)src;

  for (size_t i = 0; i < size; i++)
    *p++ = psrc[i];

  return (p - (buf + offset));
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
  assert(context_);
  // assert(func_);
  if (func_)
    sp_debug("BLOB - patch area at %lx for calling %s, will return to %lx",
             blob_, func_->name().c_str(), ret_addr);

  if (blob_size_ > 0) {
    if (func_) sp_debug("BLOB - Blob is constructed for calling %s(), just grab it!",
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
  insnsize = emit_call_abs((long)head_, blob_, offset);
  offset += insnsize;

  // 3. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  // 4. call ORIG_FUNCTION
  if (func_) {
    // Direct call
    if (ret_addr) {
      // Case 1: using call instruction
      insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
    } else {
      // Case 2: using jump instruction, for tail code optimization
      insnsize = emit_call_jump((long)func_->addr(), blob_, offset);
    }
  } else {
    // Indirect call
    insnsize = emit_call_orig((long)orig_insn_.c_str(),
                              orig_insn_.size(), blob_, offset);
  }
  offset += insnsize;

  // 1. save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;

  // 2. pass parameters
  insnsize = emit_pass_param((long)point_, (long)context_, blob_, offset);
  offset += insnsize;

  // 3. call payload
  insnsize = emit_call_abs((long)tail_, blob_, offset);
  offset += insnsize;

  // 3. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  // 6. jmp ORIG_INSN_ADDR
  if (ret_addr) {
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
