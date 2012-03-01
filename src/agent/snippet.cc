/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Platform dependent stuffs, e.g., code generation part, are put in
// Snippet-x86_64.cc and Snippet-i386.cc

#include "agent/context.h"
#include "agent/event.h"
#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"
#include "agent/patchapi/point.h"
#include "agent/snippet.h"
#include "common/utils.h"

const size_t BLOB_SIZE = 1024;
const size_t BLK_LIMIT = BLOB_SIZE - 200;

namespace sp {

	extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

  // Constructor
  // Allocate a buffer to hold generated patch area for a call site.
  SpSnippet::SpSnippet(SpFunction* f,
                       SpPoint* pt,
                       PayloadFunc entry,
                       PayloadFunc exit)
    : func_(f),
			point_(pt),
			entry_(entry),
			exit_(exit),
			blob_(NULL),
      blob_size_(0),
			spring_(NULL),
			spring_size_(0),
			spring_blk_(NULL)	{

    // TODO (wenbin): For now, just statically allocate a 1024-byte buffer,
    // should have a smarter memory allocator.
		assert(g_as);
		assert(pt);
    sp_debug("SNIPPET CONSTRUCTOR - payload entry %lx", (long)entry_);
    InitSavedRegMap();
  }

  // Destructor
  SpSnippet::~SpSnippet() {
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
  SpSnippet::BuildBlob(const size_t est_size, // The estimate blob size
                       const bool reloc) {    // Need to relocate a block?
											

		assert(point_);
		if (!blob_) {
			sp_debug("ALLOC BLOB - for size %ld", (long)est_size);
			blob_ = (char*)GetBlob(est_size);
			assert(blob_);
		}

		dt::Address ret_addr = point_->ret_addr();

    // If this blob is already generated? If so, just return it.
    if (blob_size_ > 0 && blob_) {
			sp_debug("BLOB EXIST - avoid regenerating it");
      return blob_;
    }

		SpBlock* b = point_->GetBlock();
		assert(b);

		sp_debug("RET_ADDR - is %lx for point %lx", ret_addr,
						 (dt::Address)b->last());

    // 1. Relocate call block, if it's indirect call
    if (reloc) {
			sp_debug("RELOC BLOCK");
      /*
			if (b->size() >= BLK_LIMIT) {
				sp_debug("NO SUFFICIENT SPACE - for BLOB (%lu >= %lu)",
								 b->size(), (unsigned long)BLK_LIMIT);
				return NULL;
			}
      */
      blob_size_ += reloc_block(b, blob_, blob_size_);
    }


    // 2. Save context (TODO: better to have liveness analysis)
    blob_size_ += emit_save(blob_, blob_size_);

    // 3. Call payload before function call
    long param_func = 0;
    long called_func = (long)entry_;
    sp_debug("PAYLOAD ENTRY - at %lx", called_func);
		assert(g_context);
    if (g_context->IsIpcEnabled()) {
      param_func = (long)entry_;
      called_func = (long)g_context->wrapper_entry();
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

    if (point_->tailcall() && func_) {
			sp_debug("TAIL_CALL_DIRECT_CALL ");
			assert(!ret_addr);
      // 5.1. tail call and direct call
      blob_size_ += emit_jump_abs((long)func_->addr(), blob_, blob_size_);
      goto EXIT;
    } else if (!point_->tailcall() && func_) {
			assert(ret_addr);
			sp_debug("SIMPLE_DIRECT_CALL ");
      // 5.2. non tail call and Direct call
      blob_size_ += emit_call_abs((long)func_->addr(),
																	blob_,
																	blob_size_,
																	false);
    } else {

			if (!point_->tailcall())
				sp_debug("INDIRECT_CALL ");
			else
				sp_debug("TAIL_CALL_INDIRECT_CALL");

      // 5.3. indirect call
			assert(b->orig_call_insn());
      blob_size_ += emit_call_orig(blob_, blob_size_);
    }

    if (g_context->IsIpcEnabled() || exit_) {
      // 6. save context
      blob_size_ += emit_save(blob_, blob_size_);

      // 7. Pass parameters
      param_func = 0;
      called_func = (long)exit_;
      if (g_context->IsIpcEnabled()) {
        param_func = (long)exit_;
        called_func = (long)g_context->wrapper_exit();
      }
      blob_size_ += emit_pass_param((long)point_,
																		param_func,
																		blob_,
																		blob_size_);
      blob_size_ += emit_call_abs(called_func, blob_, blob_size_, true);

      // 8. Restore context
      blob_size_ += emit_restore(blob_, blob_size_);
    }

    // 9. Jump back to ORIG_INSN_ADDR
    blob_size_ += emit_jump_abs(ret_addr, blob_, blob_size_);

  EXIT:

		if (func_) {
			sp_debug("DUMP PATCH AREA (%lu bytes) for point %lx for %s - {",
							 (unsigned long)blob_size_, b->last(),
							 func_->name().c_str());
		} else {
			sp_debug("DUMP PATCH AREA (%lu bytes) for point %lx - {",
							 (unsigned long)blob_size_, b->last());
		}
    sp_debug("%s", g_parser->DumpInsns((void*)blob_,
																			 blob_size_).c_str());
    sp_debug("}");

    return blob_;
  }

  // Relocate a block to the patch area
  size_t
  SpSnippet::reloc_block(SpBlock* blk,
												 char* buf,
												 size_t offset) {
		assert(blk);
    char* p = buf + offset;
    dt::Address call_addr = blk->last();

		ph::PatchBlock::Insns insns;
    blk->getInsns(insns);
    for (ph::PatchBlock::Insns::iterator i = insns.begin();
				 i != insns.end(); i++) {
      dt::Address a = i->first;
			in::Instruction::Ptr insn = i->second;
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
	SpBlock*
  SpSnippet::spring_blk() {
    if (spring_blk_) return spring_blk_;

		assert(point_);
    size_t min_springblk_size = jump_abs_size() * 2;
		SpBlock* callblk = point_->GetBlock();

    // Short jump is 2 bytes
		assert(callblk);
    long after_jmp = callblk->start() + 2;
    bool done = false;

		sp_debug("LOOKING FOR SPRING BOARD - springboard size should >= %ld",
						 (long)min_springblk_size);

    // Find a nearby block
		SpObject* obj = callblk->GetObject();
		assert(obj);

		pe::CodeObject* co = obj->co();
		pe::CodeSource* cs = co->cs();
    std::vector<pe::CodeRegion*> regions = cs->regions();

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
			pe::CodeRegion* cr = regions[i];
			size_t cr_low = cr->low();
			size_t cr_high = cr->high();
			sp_debug("REGION [%lx, %lx] - (%lx, %lx)", (unsigned long)cr_low,
							 (unsigned long)cr_high, lower, upper);
      if ((lower <= (long)cr_low && (long)cr_low < upper) ||
          ((long)cr_low <= lower && upper < (long)cr_high) ||
          (lower <= (long)cr_high && (long)cr_high < upper)
          ) {
        // XXX: Not any method to iterate blocks in a region?
   			dt::Address span_addr = lower;
        do {
          set<pe::Block*> blks;
          co->findBlocks(cr, span_addr, blks);
          if (blks.size() == 0) {
            ++span_addr;
            continue;
          }
					pe::Block* b;
          for (set<pe::Block*>::iterator bi = blks.begin();
							 bi != blks.end(); bi++) {
            b = *bi;
          }
					sp_debug("POTENTIAL SPRING BOARD - %ld found, [%lx, %lx]",
									 (unsigned long)blks.size(), b->start(),
									 b->size() + b->start());

          size_t rel = b->start() + jump_abs_size() - after_jmp;
					if (obj->codeBase() != 0) {
						rel += obj->load_addr();
					}

          if (!sp::IsDisp8(rel)) {
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
					SpBlock* pb = BLK_CAST(obj->getBlock(b));
					assert(pb);

          // For simplicity, we don't want call block
          if (pb->containsCall()) {
						sp_debug("CALL BLK, SKIPPED - we don't relocate call block");
            span_addr = b->end();
            continue;
          }

          // For simplicity, we don't relocate used spring block
          if (pb->IsSpring()) {
						sp_debug("SPRING BOARD, SKIPPED - we don't relocate second-hand"
										 " spring board");
            span_addr = b->end();
            continue;
          }
					sp_debug("GOT SPRING BOARD");
          spring_blk_ = pb;
          pb->SetIsSpring(true);
          done = true;
          break;
        } while ((long)span_addr < upper);
        if (done) break;
      }
    }

    return spring_blk_;
  }

  // Build the spring block
  char*
  SpSnippet::spring(SpBlock* spring_blk) {
		assert(spring_blk);
		dt::Address ret_addr = spring_blk->last();
		assert(g_as);
		SpObject* obj = point_->GetObject();
		assert(obj);
    spring_ = (char*)g_as->malloc(obj,
																	spring_blk->size() + jump_abs_size(),
																	obj->load_addr());
    spring_size_ = 0;

    // 1, Relocate spring block to the patch area
    spring_size_ += reloc_block(spring_blk_, spring_, 0);

    // 2, Jump back
    spring_size_ += emit_jump_abs(ret_addr, spring_, spring_size_);

		assert(point_->block());
    sp_debug("DUMP RELOC SPRING INSNS (%lu bytes) for point %lx- {",
						 (unsigned long)spring_size_, point_->block()->last());
    sp_debug("%s", g_parser->DumpInsns((void*)spring_, spring_size_).c_str());
    sp_debug("}");

    return spring_;
  }

  dt::Address
	SpSnippet::GetBlob(const size_t estimate_size) {
		if (estimate_size == 0) {
			// Possible to return NULL
			return (dt::Address)blob_;
		}

		assert(point_);
		SpObject* obj = point_->GetObject();
		assert(obj);

		// We may want to reallocate it, so free existing buffer first
		if (blob_) {
      sp_debug("REALLOC - for %lx", point_->block()->last());
			g_as->free(obj, (dt::Address)blob_);
		}

		blob_ = (char*)g_as->malloc(obj, estimate_size,
																obj->load_addr());
		assert(blob_);
		return (dt::Address)blob_;
	}

  bool
  SpSnippet::GetRegInternal(dt::MachRegister reg,
                            dt::Address* out) {

    if (saved_reg_map_.find(reg) == saved_reg_map_.end()) {
      return false;
    }
    
    int offset = saved_reg_map_[reg];
    *out = RegVal(offset);

    sp_debug("ORIG VAL - %s = %lx, size: %ld",
             reg.name().c_str(), *out, (long)reg.size());
    switch(reg.size()) {
      case 8:
        // Just return!
        break;
      case 4:
        // *out = (0x00000000ffffffff & *out);
        break;
      case 2:
        // *out = (0x000000000000ffff & *out);
        break;
      case 1:
        if (reg.name().find('h') != std::string::npos) {
          *out = ((0x000000000000ff00 & *out) >> 8);
        } else {
          // *out = (0x00000000000000ff & *out);
        }
        break;
      default:
        return false;
    }

    sp_debug("NEW VAL - %s = %lx",
             reg.name().c_str(), *out);
      
    return true;
  }

}
