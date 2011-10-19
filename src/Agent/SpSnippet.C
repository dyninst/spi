#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "Visitor.h"
#include "SpPoint.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;

namespace sp {

SpSnippet::SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
                     Dyninst::PatchAPI::Point* pt,
                     SpContext* c,
                     PayloadFunc head, PayloadFunc tail)
  : func_(f), point_(pt), context_(c), head_(head), tail_(tail), blob_size_(0) {

  // assert(context_ && "SpContext is NULL");
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

char* SpSnippet::blob(Dyninst::Address ret_addr, bool reloc, bool spring) {
  assert(context_);
  ret_addr_ = ret_addr;

  if (blob_size_ > 0) {
    return blob_;
  }

  size_t offset = 0;
  size_t insnsize = 0;

  // 5. relocate block
  if (reloc) {
    PatchBlock* blk = NULL;
    if (!spring) {
      blk = point_->block();
    } else {
      blk = NULL;
    }
    insnsize = reloc_block(blk, offset);
    offset += insnsize;
  }

  // 1. save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;
  if (!func_) {
    sp::SpPoint* spt = static_cast<sp::SpPoint*>(point_);
    long* l = spt->saved_context_ptr();
    insnsize = emit_save_sp((long)l, blob_, offset);
    offset += insnsize;
  }

  // 2. pass parameters
  insnsize = emit_pass_param((long)point_, blob_, offset);
  offset += insnsize;

  // 3. call head payload
  insnsize = emit_call_abs((long)head_, blob_, offset, true);
  offset += insnsize;

  // 4. restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;


  before_call_orig_ = offset;

  // 6. call ORIG_FUNCTION
  if (!ret_addr) {
    // tail call
    sp_debug("TAIL");
    insnsize = emit_jump_abs((long)func_->addr(), blob_, offset);
  } else if (func_) {
    // Direct call
    sp_debug("DIRECT");
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  } else {
    // Indirect call
    sp_debug("INDIRECT");
    insnsize = emit_call_orig((long)orig_insn_.c_str(),
                              orig_insn_.size(), blob_, offset);
  }
  offset += insnsize;
  if (tail_) {

    // 7. save context
    insnsize = emit_save(blob_, offset);
    offset += insnsize;

    // 8. pass parameters
    insnsize = emit_pass_param((long)point_, blob_, offset);
    offset += insnsize;

    // 9. call payload
    insnsize = emit_call_abs((long)tail_, blob_, offset, true);
    offset += insnsize;

    // 10. restore context
    insnsize = emit_restore(blob_, offset);
    offset += insnsize;
  }

  // 11. jmp ORIG_INSN_ADDR
  insnsize = emit_jump_abs(ret_addr, blob_, offset);

  offset += insnsize;
  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");

  return blob_;
}

void SpSnippet::fixup(PatchFunction* f) {
  // sp_debug("FIXUP - for call instructions involving PC value");

  if (!f) return;
  func_ = f;

  size_t offset = before_call_orig_;
  size_t insnsize = 0;

  // 5. call ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  offset += insnsize;

  if (tail_) {
    // 6. save context
    insnsize = emit_save(blob_, offset);
    offset += insnsize;

    // 7. pass parameters
    insnsize = emit_pass_param((long)point_, blob_, offset);
    offset += insnsize;

    // 8. call payload
    insnsize = emit_call_abs((long)tail_, blob_, offset, true);
    offset += insnsize;

    // 9. restore context
    insnsize = emit_restore(blob_, offset);
    offset += insnsize;
  }

  // 10. jmp ORIG_INSN_ADDR
  if (ret_addr_) {
    insnsize = emit_jump_abs(ret_addr_, blob_, offset);
  } else {
    // sp_debug("TAIL CALL");
    insnsize = emit_ret(blob_, offset);
  }
  offset += insnsize;
  blob_size_ = offset;

  // sp_debug("DUMP INSN (%d bytes)- {", offset);
  // sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  // sp_debug("DUMP INSN - }");
}


/*
class BlkInsnVisitor : public Visitor{
  public:
    BlkInsnVisitor() : Visitor() {}

    virtual void visit(RegisterAST* r) {
    }
    virtual void visit(BinaryFunction* b) {
    }
    virtual void visit(Immediate* i) {
    }
    virtual void visit(Dereference* d) {
    }
};
*/
size_t SpSnippet::reloc_block(Dyninst::PatchAPI::PatchBlock* blk, size_t offset) {
  char* p = blob_ + offset;
  size_t insnsize = 0;
  Dyninst::Address call_addr = blk->last();

  sp_debug("RELOC BLOCK - begin");

  PatchBlock::Insns insns;
  blk->getInsns(insns);
  for (PatchBlock::Insns::iterator i = insns.begin(); i != insns.end(); i++) {
    using namespace Dyninst::InstructionAPI;

    Dyninst::Address a = i->first;
    Instruction::Ptr insn = i->second;
    sp_debug("RELOC INSN - %lx : %s", a, context_->parser()->dump_insn((void*)a, insn->size()).c_str());

    insnsize = reloc_insn(i, call_addr, p);
    p += insnsize;

    /*
    Expression::Ptr trg = insn->getControlFlowTarget();
    BlkInsnVisitor visitor;
    trg->apply(&visitor);
    */
  }

  return (p - (blob_ + offset));
}

}
