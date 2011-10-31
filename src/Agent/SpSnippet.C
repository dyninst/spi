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
  : func_(f), point_(pt), context_(c), head_(head), tail_(tail), blob_size_(0),
    spring_size_(0), spring_blk_(NULL) {

  // assert(context_ && "SpContext is NULL");
  Dyninst::PatchAPI::PatchMgrPtr mgr = c->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  blob_ = (char*)as->malloc(pt->obj(), 1024, pt->obj()->codeBase());
}

SpSnippet::~SpSnippet() {
  free(blob_);
}

  /*
size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset) {
  char* p = buf + offset;
  char* psrc = (char*)src;

  for (size_t i = 0; i < size; i++)
    *p++ = psrc[i];

  return (p - (buf + offset));
}
  */

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

  sp_debug("ret_addr: %d, reloc: %d, spring: %d", ret_addr, reloc, spring);
  // 5. relocate call block
  if (reloc) {
    PatchBlock* blk = NULL;
    blk = point_->block();
    insnsize = reloc_block(blk, blob_, offset);
    offset += insnsize;
  }

  // 1. save context
  insnsize = emit_save(blob_, offset, reloc);
  offset += insnsize;
  sp_debug("func_: %lx", func_);
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
  insnsize = emit_restore(blob_, offset, reloc);
  offset += insnsize;

  // before_call_orig_ = offset;

  // 6. call ORIG_FUNCTION
  // There are four cases need to consider:
  // Case 1: tail call and direct call
  // Case 2: non tail call and direct call
  // Case 3: indirect call (tail call or non tail call)
  //  - x86-32: just relocate the orig insn
  //  - x86-64: relocate the orig insn for non-pc-sensitive insn
  //            emulate the insn for pc-sensitive insn
  if (!ret_addr && func_) {
    // tail call and direct call
    sp_debug("TAIL to %s", func_->name().c_str());
    insnsize = emit_jump_abs((long)func_->addr(), blob_, offset);
    goto EXIT;
  } else if (ret_addr && func_) {
    // non tail call and Direct call
    sp_debug("DIRECT");
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  } else {
    // indirect call
    sp_debug("INDIRECT");
    insnsize = emit_call_orig((long)orig_insn_.c_str(),
                              orig_insn_.size(), blob_, offset,
                              /*tail call?*/(ret_addr == 0));
  }

  offset += insnsize;
  if (tail_) {

    // 7. save context
    insnsize = emit_save(blob_, offset, reloc);
    offset += insnsize;

    // 8. pass parameters
    insnsize = emit_pass_param((long)point_, blob_, offset);
    offset += insnsize;

    // 9. call payload
    insnsize = emit_call_abs((long)tail_, blob_, offset, true);
    offset += insnsize;

    // 10. restore context
    insnsize = emit_restore(blob_, offset, reloc);
    offset += insnsize;
  }

  // 11. jmp ORIG_INSN_ADDR
  insnsize = emit_jump_abs(ret_addr, blob_, offset);

EXIT:
  offset += insnsize;
  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");

  return blob_;
}

void SpSnippet::fixup(PatchFunction* f) {
  sp_debug("FIXUP - for call instructions involving PC value");
  sp_debug("BEFORE FIXUP DUMP INSN (%d bytes)- {", blob_size_);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, blob_size_).c_str());
  sp_debug("DUMP INSN - }");

  if (!f) return;
  func_ = f;

  size_t offset = before_call_orig_;
  size_t insnsize = 0;

  // 5. call ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  //offset += insnsize;
  //blob_size_ = offset;

  sp_debug("AFTER FIXUP DUMP INSN (%d bytes)- {", blob_size_);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, blob_size_).c_str());
  sp_debug("DUMP INSN - }");
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
size_t SpSnippet::reloc_block(Dyninst::PatchAPI::PatchBlock* blk, char* buf, size_t offset) {
  char* p = buf + offset;
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
  }

  return (p - (buf + offset));
}

PatchBlock* SpSnippet::spring_blk() {
  if (spring_blk_) return spring_blk_;

  size_t min_springblk_size = jump_abs_size() * 2;
  PatchBlock* callblk = point_->block();
  long after_jmp = callblk->start() + 2; // short jump 2 bytes

  // Find a nearby block
  // XXX: this algorithm sucks! Need a smarter one.
  size_t rel = 0;
  PatchBlock* springblk = NULL;
  bool done = false;
  Dyninst::PatchAPI::PatchObject* obj = callblk->obj();
  std::vector<PatchBlock*> blks;
  using namespace Dyninst::ParseAPI;
  CodeObject* co = obj->co();
  CodeObject::funclist& funcs = co->funcs();
  for (CodeObject::funclist::iterator fi = funcs.begin(); fi != funcs.end(); fi++) {
    PatchFunction* f = obj->getFunc(*fi);
    const PatchFunction::Blockset& blks = f->getAllBlocks();
    for (PatchFunction::Blockset::iterator bi = blks.begin(); bi != blks.end(); bi++) {
      PatchBlock* b = *bi;
      // For simplicity, we don't want call block
      if (b->containsCall()) continue;
      // For simplicity, we don't relocate used spring block
      if (context_->in_spring_set(b)) continue;
      rel = b->start() + jump_abs_size() - after_jmp;
      // A qualified spring block should be within 128 bytes from current call block, and
      // should be able to hold two absolute jumps before the last insn in that block -
      // this simplifies our relocation
      size_t s = b->last() - b->start();
      sp_debug("CANDIDATE - size %d, rel: %d", s, std::abs((long)rel));
      if (std::abs((long)rel) <= 128 && s >= min_springblk_size) {
        sp_debug("GOT SPRING - at %lx, relative addr from %lx is %lx, size %d >= %d (two abs jumps)",
                 b->start(), callblk->start(), rel, s, min_springblk_size);
        springblk = b;
        context_->add_spring(b);
        break;
      }
    } // for each block
    if (springblk) break;
  } // for each function

  spring_blk_ = springblk;
  return springblk;
}

char* SpSnippet::spring(Dyninst::Address ret_addr) {
  Dyninst::PatchAPI::PatchMgrPtr mgr = context_->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  spring_ = (char*)as->malloc(point_->obj(), 1024, point_->obj()->codeBase());

  // 1, relocate spring block
  size_t offset = 0;
  size_t insnsize = reloc_block(spring_blk_, spring_, 0);
  offset += insnsize;

  sp_debug("DONE RELOC - w/ offset: %d", offset);

  // 2, jump back
  insnsize = emit_jump_abs(ret_addr, spring_, offset);
  offset += insnsize;

  spring_size_ = offset;

  sp_debug("DUMP RELOC SPRING INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)spring_, spring_size_).c_str());
  sp_debug("DUMP INSN - }");

  return spring_;
}

}
