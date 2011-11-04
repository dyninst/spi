#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "Visitor.h"
#include "SpPoint.h"
#include "SpUtils.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::Point;

namespace sp {

SpSnippet::SpSnippet(PatchFunction* f,
                     Point* pt,
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

// The core part of code generation for the patch area
char* SpSnippet::blob(Dyninst::Address ret_addr, bool reloc, bool spring) {
  assert(context_);
  ret_addr_ = ret_addr;

  if (blob_size_ > 0) {
    return blob_;
  }

  size_t offset = 0;
  size_t insnsize = 0;

  //sp_debug("ret_addr: %lx, reloc: %d, spring: %d", ret_addr, reloc, spring);
  //-------------------------------------------
  // 1. Relocate call block, if it's indirect call
  //-------------------------------------------
  if (reloc) {
    PatchBlock* blk = NULL;
    blk = point_->block();
    insnsize = reloc_block(blk, blob_, offset);
    offset += insnsize;
  }

  //-------------------------------------------
  // 2. Save context
  //-------------------------------------------
  insnsize = emit_save(blob_, offset, reloc);
  offset += insnsize;
  //sp_debug("func_: %lx", func_);
  if (!func_||reloc) {
    sp::SpPoint* spt = static_cast<sp::SpPoint*>(point_);
    long* l = spt->saved_context_ptr();
    insnsize = emit_save_sp((long)l, blob_, offset);
    offset += insnsize;
  }

  //-------------------------------------------
  // 3. Pass parameter
  //-------------------------------------------
  insnsize = emit_pass_param((long)point_, blob_, offset);
  offset += insnsize;

  //-------------------------------------------
  // 4. Call head payload
  //-------------------------------------------
  insnsize = emit_call_abs((long)head_, blob_, offset, true);
  offset += insnsize;

  //-------------------------------------------
  // 5. Restore context
  //-------------------------------------------
  insnsize = emit_restore(blob_, offset, reloc);
  offset += insnsize;

  //-------------------------------------------
  // 6. call ORIG_FUNCTION
  // There are four cases need to consider:
  // Case 1: tail call and direct call
  // Case 2: non tail call and direct call
  // Case 3: indirect call (tail call or non tail call)
  //-------------------------------------------
  if (!ret_addr && func_) {
    // 6.1. tail call and direct call
    //sp_debug("TAIL to %s", func_->name().c_str());
    insnsize = emit_jump_abs((long)func_->addr(), blob_, offset);
    goto EXIT;
  } else if (ret_addr && func_) {
    // 6.2. non tail call and Direct call
    //sp_debug("DIRECT");
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  } else {
    // 6.3. indirect call
    //sp_debug("INDIRECT");
    insnsize = emit_call_orig((long)point_->block()->last(),
                              orig_call_insn_->size(), blob_, offset);
  }
  offset += insnsize;

  if (tail_) {
    //-------------------------------------------
    // 7. save context
    //-------------------------------------------
    insnsize = emit_save(blob_, offset, reloc);
    offset += insnsize;

    //-------------------------------------------
    // 8. pass parameters
    //-------------------------------------------
    insnsize = emit_pass_param((long)point_, blob_, offset);
    offset += insnsize;

    //-------------------------------------------
    // 9. call payload
    //-------------------------------------------
    insnsize = emit_call_abs((long)tail_, blob_, offset, true);
    offset += insnsize;

    //-------------------------------------------
    // 10. restore context
    //-------------------------------------------
    insnsize = emit_restore(blob_, offset, reloc);
    offset += insnsize;
  }

  //-------------------------------------------
  // 11. jmp ORIG_INSN_ADDR
  //-------------------------------------------
  insnsize = emit_jump_abs(ret_addr, blob_, offset);

EXIT:
  offset += insnsize;
  blob_size_ = offset;
  /*
  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");
  */
  return blob_;
}

// Relocate a block
size_t SpSnippet::reloc_block(PatchBlock* blk, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;
  Dyninst::Address call_addr = blk->last();

  //sp_debug("RELOC BLOCK - begin");

  PatchBlock::Insns insns;
  blk->getInsns(insns);
  for (PatchBlock::Insns::iterator i = insns.begin(); i != insns.end(); i++) {
    using namespace Dyninst::InstructionAPI;

    Dyninst::Address a = i->first;
    Instruction::Ptr insn = i->second;
    //sp_debug("RELOC INSN - %lx : %s", a, context_->parser()->dump_insn((void*)a, insn->size()).c_str());
    insnsize = reloc_insn(a, insn, call_addr, p);
    p += insnsize;
  }

  return (p - (buf + offset));
}

// Find and return a spring block
// For now, the algorithm looks for a spring block conservatively. 
// A spring block returned should fulfill all the requirements:
//
// 1. Only in the same object of the call block
// 2. not a call block itself
// 3. large enough to host two absolute jump
// 4. close enough to the call block (within 128-byte)
//
// We iterate through all CodeRegions, and get then narrow down the search
// space in one, two, or up to three CodeRegions. 
PatchBlock* SpSnippet::spring_blk() {
  if (spring_blk_) return spring_blk_;

  size_t min_springblk_size = jump_abs_size() * 2;
  PatchBlock* callblk = point_->block();
  long after_jmp = callblk->start() + 2; // short jump 2 bytes
  bool done = false;

  // Find a nearby block
  PatchBlock* springblk = NULL;
  Dyninst::PatchAPI::PatchObject* obj = callblk->obj();

  using namespace Dyninst::ParseAPI;
  CodeObject* co = obj->co();
  CodeSource* cs = co->cs();
  std::vector<CodeRegion*> regions = cs->regions();
  // sp_debug("REGION - %d regions found", regions.size());

  // -128 <= b->start() + jump_abs_size() - after_jmp < 127
  long upper = 127 + after_jmp - jump_abs_size();
  long lower = -128 + after_jmp - jump_abs_size();
  // sp_debug("RANGE - upper: %lx, lower: %lx; jump from %lx", upper, lower, after_jmp);

  for (int i = 0; i < regions.size(); i++) {
    CodeRegion* cr = regions[i];
    // sp_debug("REGION %d - low: %lx, high: %lx", i, cr->low(), cr->high());
    if ((lower <= cr->low() && cr->low() < upper) ||
        (cr->low() <= lower && upper < cr->high()) ||
        (lower <= cr->high() && cr->high() < upper)
       ) {
      // XXX: Not any method to iterate blocks in a region?
      Dyninst::Address span_addr = lower;
      do {
        set<Block*> blks;
        co->findBlocks(cr, span_addr, blks);
        if (blks.size() == 0) {
          ++span_addr;
          continue;
        }
        Block* b;
        for (set<Block*>::iterator bi = blks.begin(); bi != blks.end(); bi++) {
          b = *bi;
          // sp_debug("BLK - low: %lx, high: %lx; callblock(%lx, %lx)",
          //         (*bi)->start(), (*bi)->end(), callblk->start(), callblk->end());
        }
        size_t rel = b->start() + jump_abs_size() - after_jmp;
        if (!sp::is_disp8(rel)) {
          // sp_debug("TOO FAR AWAY");
          span_addr = b->end();
          continue;
        }
        size_t s = b->lastInsnAddr() - b->start();
        if (s < min_springblk_size) {
          // sp_debug("TOO SMALL - %d < %d", s, min_springblk_size);
          span_addr = b->end();
          continue;
        }
        // sp_debug("CANDIDATE - size %d, rel: %d", s, (long)rel);
        PatchBlock* pb = obj->getBlock(b);
        // For simplicity, we don't want call block
        if (pb->containsCall()) {
          // sp_debug("HAS CALL");
          span_addr = b->end();
          continue;
        }

        // For simplicity, we don't relocate used spring block
        if (context_->in_spring_set(pb)) {
          // sp_debug("ALREADY A SPRING BLK");
          span_addr = b->end();
          continue;
        }
        // sp_debug("GOT IT");
        springblk = pb;
        context_->add_spring(pb);
        done = true;
        break;
      } while (span_addr < upper);
      if (done) break;
    }
  }

  spring_blk_ = springblk;
  return springblk;
}

// Build the spring block
char* SpSnippet::spring(Dyninst::Address ret_addr) {
  Dyninst::PatchAPI::PatchMgrPtr mgr = context_->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  spring_ = (char*)as->malloc(point_->obj(), 1024, point_->obj()->codeBase());

  // 1, relocate spring block
  size_t offset = 0;
  size_t insnsize = reloc_block(spring_blk_, spring_, 0);
  offset += insnsize;

  // sp_debug("DONE RELOC - w/ offset: %d", offset);

  // 2, jump back
  insnsize = emit_jump_abs(ret_addr, spring_, offset);
  offset += insnsize;

  spring_size_ = offset;

  // sp_debug("DUMP RELOC SPRING INSN (%d bytes)- {", offset);
  // sp_debug("%s", context_->parser()->dump_insn((void*)spring_, spring_size_).c_str());
  // sp_debug("DUMP INSN - }");

  return spring_;
}

}
