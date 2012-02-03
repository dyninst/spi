#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "SpPoint.h"
#include "SpUtils.h"
#include "SpObject.h"

using ph::Point;
using dt::Address;
using ph::PatchBlock;
using ph::PatchFunction;

const size_t BLOB_SIZE = 1024;
const size_t BLK_LIMIT = BLOB_SIZE - 200;

namespace sp {

  // Constructor
  // Allocate a buffer to hold generated patch area for a call site.
  SpSnippet::SpSnippet(PatchFunction* f,
                       SpPoint* pt,
                       SpContext* c,
                       PayloadFunc entry,
                       PayloadFunc exit)
    : func_(f), point_(pt), context_(c), entry_(entry), exit_(exit),
      blob_size_(0), spring_size_(0), spring_blk_(NULL), realloc_(false) {

    ph::PatchMgrPtr mgr = c->parser()->mgr();
    ph::AddrSpace* as = mgr->as();

    // TODO (wenbin): For now, just statically allocate a 1024-byte buffer,
    // should have a smarter memory allocator.
    blob_ = (char*)as->malloc(pt->obj(), BLOB_SIZE,
                   static_cast<sp::SpObject*>(pt->obj())->load_addr());
  }

  // Destructor
  SpSnippet::~SpSnippet() {
    if (!realloc_) free(blob_);
  }

  char* SpSnippet::realloc() {
    char* buf = NULL;
    realloc_ = true;

    return buf;
  }

  // The core part of code generation for the patch area. The basic logic:
  // 1. Relocate call block, if it's indirect call;
  // 2. Save context (TODO: better to have liveness analysis);
  // 3. Call payload before function call;
  // 4. Restore context;
  // 5. call ORIG_FUNCTION;
  // 6. save context;
  // 7. Pass parameters;
  // 8. Restore context;
  // 9. Jump back to ORIG_INSN_ADDR.
  char*
  SpSnippet::blob(bool reloc, bool spring) {
    assert(context_);

		Address ret_addr = point_->ret_addr();
    // If this blob is already generated? If so, just return it.
    if (blob_size_ > 0) {
      return blob_;
    }

		sp_debug("RET_ADDR - is %lx for point %lx", ret_addr,
						 (Address)point_->block()->last());

    // 1. Relocate call block, if it's indirect call
    if (reloc) {
			sp_debug("RELOC BLOCK");
      PatchBlock* blk = NULL;
      blk = point_->block();
			if (blk->size() >= BLK_LIMIT) {
				sp_debug("NO SUFFICIENT SPACE - for BLOB (%lu >= %lu)",
								 blk->size(), (unsigned long)BLK_LIMIT);
				return NULL;
			}
      blob_size_ += reloc_block(blk, blob_, blob_size_);
    }


    // 2. Save context (TODO: better to have liveness analysis)
    blob_size_ += emit_save(blob_, blob_size_);

    // 3. Call payload before function call
    long param_func = 0;
    long called_func = (long)entry_;
    if (context_->allow_ipc()) {
      param_func = (long)entry_;
      called_func = (long)context_->wrapper_entry();
    }
    blob_size_ += emit_pass_param((long)point_, param_func, blob_,
																	blob_size_);
    blob_size_ += emit_call_abs(called_func, blob_, blob_size_, true);

    // 4. Restore context
    blob_size_ += emit_restore(blob_, blob_size_);

    // 5. Call ORIG_FUNCTION
    // There are four cases need to consider:
    // Case 1: tail call and direct call
    // Case 2: non tail call and direct call
    // Case 3: indirect call (tail call or non tail call)
    if (!ret_addr && func_) {
			sp_debug("TAIL_CALL_DIRECT_CALL ");
      // 5.1. tail call and direct call
      blob_size_ += emit_jump_abs((long)func_->addr(), blob_, blob_size_);
      goto EXIT;
    } else if (ret_addr && func_) {
			sp_debug("SIMPLE_DIRECT_CALL ");
      // 5.2. non tail call and Direct call
      blob_size_ += emit_call_abs((long)func_->addr(), blob_,
																	blob_size_, false);
    } else {
			if (ret_addr)	sp_debug("INDIRECT_CALL ");
			else sp_debug("TAIL_CALL_INDIRECT_CALL");

      // 5.3. indirect call
			assert(point_->orig_call_insn());
      blob_size_ += emit_call_orig(blob_, blob_size_);
    }

    if (context_->allow_ipc() || exit_) {
      // 6. save context
      blob_size_ += emit_save(blob_, blob_size_);

      // 7. Pass parameters
      param_func = 0;
      called_func = (long)exit_;
      if (context_->allow_ipc()) {
        param_func = (long)exit_;
        called_func = (long)context_->wrapper_exit();
      }
      blob_size_ += emit_pass_param((long)point_, param_func,
																		blob_, blob_size_);
      blob_size_ += emit_call_abs(called_func, blob_, blob_size_, true);

      // 8. Restore context
      blob_size_ += emit_restore(blob_, blob_size_);
    }

    // 9. Jump back to ORIG_INSN_ADDR
    blob_size_ += emit_jump_abs(ret_addr, blob_, blob_size_);

  EXIT:

#ifndef SP_RELEASE
		if (func_) {
			sp_debug("DUMP PATCH AREA (%lu bytes) for point %lx for %s - {",
							 (unsigned long)blob_size_, point_->block()->last(),
							 func_->name().c_str());
		} else {
			sp_debug("DUMP PATCH AREA (%lu bytes) for point %lx - {",
							 (unsigned long)blob_size_, point_->block()->last());
		}
    sp_debug("%s", context_->parser()->dump_insn((void*)blob_,
																								 blob_size_).c_str());
    sp_debug("}");
#endif

    return blob_;
  }

  // Relocate a block to the patch area
  size_t
  SpSnippet::reloc_block(PatchBlock* blk, char* buf, size_t offset) {
    char* p = buf + offset;
    Address call_addr = blk->last();

    PatchBlock::Insns insns;
    blk->getInsns(insns);
    for (PatchBlock::Insns::iterator i = insns.begin();
				 i != insns.end(); i++) {
      using namespace Dyninst::InstructionAPI;

      Address a = i->first;
      Instruction::Ptr insn = i->second;
      p += reloc_insn(a, insn, call_addr, p);
    }

    return (p - (buf + offset));
  }

  // Find and return a spring block
  // For now, the algorithm looks for a spring block conservatively.
  // A spring block returned should fulfill all the requirements:
  // 1. Only in the same object of the call block
  // 2. not a call block itself
  // 3. large enough to host two absolute jump
  // 4. close enough to the call block (within 128-byte)
  // We iterate through all CodeRegions, and get then narrow down the search
  // space in one, two, or up to three CodeRegions.
  PatchBlock*
  SpSnippet::spring_blk() {
    if (spring_blk_) return spring_blk_;

    size_t min_springblk_size = jump_abs_size() * 2;
    PatchBlock* callblk = point_->block();

    // Short jump is 2 bytes
    long after_jmp = callblk->start() + 2;
    bool done = false;

		sp_debug("LOOKING FOR SPRING BOARD - springboard size should >= %ld",
						 (long)min_springblk_size);

    // Find a nearby block
    PatchBlock* springblk = NULL;
		SpObject* obj = static_cast<SpObject*>(callblk->obj());

    using namespace Dyninst::ParseAPI;
    CodeObject* co = obj->co();
    CodeSource* cs = co->cs();
    std::vector<CodeRegion*> regions = cs->regions();

    // -128 <= b->start() + jump_abs_size() - after_jmp < 127
    long upper = 127 + after_jmp - jump_abs_size();
    long lower = -128 + after_jmp - jump_abs_size();

		if (obj->codeBase() != 0) {
			upper -= obj->load_addr();
			lower -= obj->load_addr();
		}
		sp_debug("SAME OBJ - codebase-%lx, load_addr-%lx, range-[%lx, %lx) ",
						 obj->codeBase(), obj->load_addr(), lower, upper);

    for (unsigned i = 0; i < regions.size(); i++) {
      CodeRegion* cr = regions[i];
			size_t cr_low = cr->low();
			size_t cr_high = cr->high();
			sp_debug("REGION [%lx, %lx] - (%lx, %lx)", (unsigned long)cr_low,
							 (unsigned long)cr_high, lower, upper);
      if ((lower <= (long)cr_low && (long)cr_low < upper) ||
          ((long)cr_low <= lower && upper < (long)cr_high) ||
          (lower <= (long)cr_high && (long)cr_high < upper)
          ) {
        // XXX: Not any method to iterate blocks in a region?
   			Address span_addr = lower;
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
					sp_debug("POTENTIAL SPRING BOARD - %ld found, [%lx, %lx]",
									 (unsigned long)blks.size(), b->start(), b->size() + b->start());

          size_t rel = b->start() + jump_abs_size() - after_jmp;
					if (obj->codeBase() != 0) {
						rel += obj->load_addr();
					}

          if (!sp::is_disp8(rel)) {
						sp_debug("TOO BIG DISP, SKIPPED - disp=%ld", (long)rel);
            span_addr = b->end();
            continue;
          }
          size_t s = b->lastInsnAddr() - b->start();
          if (s < min_springblk_size) {
						sp_debug("TOO SMALL, SKIPPED - size=%ld", (long)s);
            span_addr = b->end();
            continue;
          }
          PatchBlock* pb = obj->getBlock(b);

          // For simplicity, we don't want call block
          if (pb->containsCall()) {
						sp_debug("CALL BLK, SKIPPED - we don't relocate call block");
            span_addr = b->end();
            continue;
          }

          // For simplicity, we don't relocate used spring block
          if (context_->in_spring_set(pb)) {
						sp_debug("SPRING BOARD, SKIPPED - we don't relocate second-hand"
										 " spring board");
            span_addr = b->end();
            continue;
          }
          springblk = pb;
          context_->add_spring(pb);
          done = true;
          break;
        } while ((long)span_addr < upper);
        if (done) break;
      }
    }

    spring_blk_ = springblk;
    return springblk;
  }

  // Build the spring block
  char*
  SpSnippet::spring(Address ret_addr) {
    ph::PatchMgrPtr mgr = context_->parser()->mgr();
    ph::AddrSpace* as = mgr->as();
    spring_ = (char*)as->malloc(point_->obj(), BLOB_SIZE, static_cast<sp::SpObject*>(point_->obj())->load_addr());
    spring_size_ = 0;

    // 1, Relocate spring block to the patch area
    spring_size_ += reloc_block(spring_blk_, spring_, 0);

    // 2, Jump back
    spring_size_ += emit_jump_abs(ret_addr, spring_, spring_size_);

#ifndef SP_RELEASE
    sp_debug("DUMP RELOC SPRING INSNS (%lu bytes) for point %lx- {", (unsigned long)spring_size_, point_->block()->last());
    sp_debug("%s", context_->parser()->dump_insn((void*)spring_, spring_size_).c_str());
    sp_debug("}");
#endif
    return spring_;
  }


}
