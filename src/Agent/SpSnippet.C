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

  assert(context_ && "SpContext is NULL");
  Dyninst::PatchAPI::PatchMgrPtr mgr = c->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  blob_ = (char*)as->malloc(pt->obj(), 1024, pt->obj()->codeBase());
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


/* The generated code:
   1. Save context
   2. Pass parameter to head payload
   3. Call head payload function
   4. Restore context
   5. Call original function
   6. Save context
   7. Pass parameter to tail payload
   8. Call tail payload function
   9. Restore context
   10. Jump back original address
 */

char* SpSnippet::blob(Dyninst::Address ret_addr) {
  assert(context_);

  ret_addr_ = ret_addr;

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

  // 3. call head payload
  insnsize = emit_call_abs((long)head_, blob_, offset, true);
  offset += insnsize;

  // 4. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  before_call_orig_ = offset;

  // 5. call ORIG_FUNCTION
  if (func_) {
    // Direct call
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  } else {
    // Indirect call
    insnsize = emit_call_orig((long)orig_insn_.c_str(),
                              orig_insn_.size(), blob_, offset);
  }
  offset += insnsize;

  // 6. save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;

  // 7. pass parameters
  insnsize = emit_pass_param((long)point_, (long)context_, blob_, offset);
  offset += insnsize;

  // 8. call payload
  insnsize = emit_call_abs((long)tail_, blob_, offset, true);
  offset += insnsize;

  // 9. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  // 10. jmp ORIG_INSN_ADDR
  if (ret_addr) {
    insnsize = emit_jump_abs(ret_addr, blob_, offset);
  } else {
    sp_debug("TAIL CALL");
    insnsize = emit_ret(blob_, offset);
  }
  offset += insnsize;
  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");

  return blob_;
}

void SpSnippet::fixup(PatchFunction* f) {
  sp_debug("FIXUP - for call instructions involving PC value");

  if (!f) return;
  func_ = f;

  size_t offset = before_call_orig_;
  size_t insnsize = 0;

  // 5. call ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  offset += insnsize;

  // 6. save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;

  // 7. pass parameters
  insnsize = emit_pass_param((long)point_, (long)context_, blob_, offset);
  offset += insnsize;

  // 8. call payload
  insnsize = emit_call_abs((long)tail_, blob_, offset, true);
  offset += insnsize;

  // 9. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;

  // 10. jmp ORIG_INSN_ADDR
  if (ret_addr_) {
    insnsize = emit_jump_abs(ret_addr_, blob_, offset);
  } else {
    sp_debug("TAIL CALL");
    insnsize = emit_ret(blob_, offset);
  }
  offset += insnsize;
  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");
}

}
