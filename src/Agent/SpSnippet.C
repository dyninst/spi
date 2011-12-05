#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "Visitor.h"
#include "SpPoint.h"
#include "SpUtils.h"
#include "SpObject.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::Point;

namespace sp {

SpSnippet::SpSnippet(PatchFunction* f,
                     Point* pt,
                     SpContext* c,
                     PayloadFunc before, PayloadFunc after)
  : func_(f), point_(pt), context_(c), before_(before), after_(after), blob_size_(0),
    spring_size_(0), spring_blk_(NULL) {

  Dyninst::PatchAPI::PatchMgrPtr mgr = c->mgr();
  Dyninst::PatchAPI::AddrSpace* as = mgr->as();
  blob_ = (char*)as->malloc(pt->obj(), 1024, static_cast<sp::SpObject*>(pt->obj())->load_addr());
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

  //-------------------------------------------
  // 3. Call payload before function call
  //-------------------------------------------
  long param_func = 0;
  long called_func = (long)before_;
  if (context_->allow_ipc()) {
    param_func = (long)before_;
    called_func = (long)context_->wrapper_before();
  }
  insnsize = emit_pass_param((long)point_, param_func, blob_, offset);
  offset += insnsize;
  insnsize = emit_call_abs(called_func, blob_, offset, true);
  offset += insnsize;

  //-------------------------------------------
  // 4. Restore context
  //-------------------------------------------
  insnsize = emit_restore(blob_, offset, reloc);
  offset += insnsize;

  //-------------------------------------------
  // 5. call ORIG_FUNCTION
  // There are four cases need to consider:
  // Case 1: tail call and direct call
  // Case 2: non tail call and direct call
  // Case 3: indirect call (tail call or non tail call)
  //-------------------------------------------
  if (!ret_addr && func_) {
    // 5.1. tail call and direct call
    insnsize = emit_jump_abs((long)func_->addr(), blob_, offset);
    goto EXIT;
  } else if (ret_addr && func_) {
    // 5.2. non tail call and Direct call
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset, false);
  } else {
    // 5.3. indirect call
    insnsize = emit_call_orig((long)point_->block()->last(),
                              orig_call_insn_->size(), blob_, offset);
  }
  offset += insnsize;

  if (context_->allow_ipc() || after_) {
    //-------------------------------------------
    // 6. save context
    //-------------------------------------------
    insnsize = emit_save(blob_, offset, reloc);
    offset += insnsize;

    //-------------------------------------------
    // 7. pass parameters
    //-------------------------------------------
    param_func = 0;
    called_func = (long)after_;
    if (context_->allow_ipc()) {
      param_func = (long)before_;
      called_func = (long)context_->wrapper_after();
    }
    insnsize = emit_pass_param((long)point_, param_func, blob_, offset);
    offset += insnsize;
    insnsize = emit_call_abs(called_func, blob_, offset, true);
    offset += insnsize;

    //-------------------------------------------
    // 8. restore context
    //-------------------------------------------
    insnsize = emit_restore(blob_, offset, reloc);
    offset += insnsize;
  }

  //-------------------------------------------
  // 9. jmp ORIG_INSN_ADDR
  //-------------------------------------------
  insnsize = emit_jump_abs(ret_addr, blob_, offset);

EXIT:
  offset += insnsize;
  blob_size_ = offset;

#ifndef SP_RELEASE
  sp_debug("DUMP PATCH AREA (%d bytes) for point %lx - {", offset, point_->block()->last());
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("}");
#endif

  return blob_;
}

// Relocate a block
size_t SpSnippet::reloc_block(PatchBlock* blk, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;
  Dyninst::Address call_addr = blk->last();

  PatchBlock::Insns insns;
  blk->getInsns(insns);
  for (PatchBlock::Insns::iterator i = insns.begin(); i != insns.end(); i++) {
    using namespace Dyninst::InstructionAPI;

    Dyninst::Address a = i->first;
    Instruction::Ptr insn = i->second;
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

  // -128 <= b->start() + jump_abs_size() - after_jmp < 127
  long upper = 127 + after_jmp - jump_abs_size();
  long lower = -128 + after_jmp - jump_abs_size();

  for (int i = 0; i < regions.size(); i++) {
    CodeRegion* cr = regions[i];
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
        }
        size_t rel = b->start() + jump_abs_size() - after_jmp;
        if (!sp::is_disp8(rel)) {
          span_addr = b->end();
          continue;
        }
        size_t s = b->lastInsnAddr() - b->start();
        if (s < min_springblk_size) {
          span_addr = b->end();
          continue;
        }
        PatchBlock* pb = obj->getBlock(b);
        // For simplicity, we don't want call block
        if (pb->containsCall()) {
          span_addr = b->end();
          continue;
        }

        // For simplicity, we don't relocate used spring block
        if (context_->in_spring_set(pb)) {
          span_addr = b->end();
          continue;
        }
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
  spring_ = (char*)as->malloc(point_->obj(), 1024, static_cast<sp::SpObject*>(point_->obj())->load_addr());

  // 1, relocate spring block
  size_t offset = 0;
  size_t insnsize = reloc_block(spring_blk_, spring_, 0);
  offset += insnsize;

  // 2, jump back
  insnsize = emit_jump_abs(ret_addr, spring_, offset);
  offset += insnsize;

  spring_size_ = offset;

#ifndef SP_RELEASE
  sp_debug("DUMP RELOC SPRING INSNS (%d bytes) for point %s- {", offset, point_->block()->last());
  sp_debug("%s", context_->parser()->dump_insn((void*)spring_, spring_size_).c_str());
  sp_debug("}");
#endif
  return spring_;
}


}
