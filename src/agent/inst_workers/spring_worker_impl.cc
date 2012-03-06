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

#include <sys/mman.h>

#include "agent/context.h"
#include "agent/inst_workers/spring_worker_impl.h"
#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"

namespace sp {
  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

  bool
  SpringboardWorker::run(SpPoint* pt) {
    sp_debug("SPRINGBOARD WORKER - runs");
    return install(pt);
  }

  bool
  SpringboardWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  SpringboardWorker::save(SpPoint* pt) {
    assert(pt);
		SpBlock* b = pt->GetBlock();
		assert(b);

		sp_debug("SPRING WORKER - saves");
		bool ret = false;

		assert(pt->snip());
		ph::PatchBlock* springblk = pt->snip()->FindSpringboard();
    if (!springblk) {
      sp_debug("NO SPRING BOARD - cannot find suitable spring board");
      return false;
    }
		SpBlock* sblk = BLK_CAST(springblk);
		assert(sblk);
		ret = sblk->save();

		assert(pt->GetBlock());
		ret = (ret && pt->GetBlock()->save());

		return ret;
  }

  bool
  SpringboardWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* callblk = pt->GetBlock();
		assert(callblk);

    SpSnippet::ptr snip = pt->snip();
		assert(snip);

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", g_parser->DumpInsns((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");

    // Find a spring block. A spring block should:
    // - big enough to hold two absolute jumps
    // - close enough to short jump from call block
    // if we cannot find one available spring block, just use trap or
    // ignore this call
		SpBlock* springblk = snip->FindSpringboard();
    if (!springblk) {
			sp_debug("FAILED TO GET A SPRINGBOARD BLOCK");
			return false;
		}

    sp_debug("BEFORE INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", g_parser->DumpInsns((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");

    // Build blob & change the permission of snippet
		size_t est_size = EstimateBlobSize(pt);
    char* blob = snip->BuildBlob(est_size,
																	/*reloc=*/true);
		if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}
		SpObject* obj = callblk->GetObject();
		assert(obj);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (!g_as->SetMemoryPermission((dt::Address)blob,
                                   snip->GetBlobSize(),
                                   perm)) {
      sp_debug("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }


    // Relocate spring block & change the permission of the relocated
    // spring block
    char* spring = snip->RelocateSpring(springblk);
		if (!spring) {
			sp_debug("FAILED TO RELOCATE SPRINGBOARD BLOCK");
			return false;
		}
    obj = springblk->GetObject();
    if (!g_as->SetMemoryPermission((dt::Address)spring,
                                   snip->GetRelocSpringSize(),
                                   perm)) {
      sp_debug("MPROTECT - Failed to change memory access permission"
               " for relocated spring blk at %lx", (dt::Address)spring);
      // g_as->dump_mem_maps();
      exit(0);
    }


    // Handle spring block
    // Write two "jump" instruction to spring block
    char springblk_insn[64];

    // First jump to relocated spring block
    size_t off = 0;
    size_t isize = snip->emit_jump_abs((long)spring, springblk_insn,
                                       off, /*abs=*/true);
    off += isize;
    size_t call_blk_jmp_trg = off;
    off = snip->emit_jump_abs((long)blob, springblk_insn, off,
                              /*abs=*/true);
    off += isize;

    // Second jump to relocated spring block
    char* addr = (char*)springblk->start();
    if (g_as->SetMemoryPermission((dt::Address)addr, springblk->size(), perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)springblk_insn, off);
    } else {
      sp_debug("MPROTECT - Failed to change memory access permission");
    }

    // Handle call block
    // Write a "jump" instruction to call block
    addr = (char*)callblk->start();
    char callblk_insn[2];
    callblk_insn[0] = 0xeb;
    callblk_insn[1] = (char)(springblk->start() + call_blk_jmp_trg
                             - ((long)addr + 2));

    if (g_as->SetMemoryPermission((dt::Address)addr, callblk->size(), perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)callblk_insn, 2);
    } else {
      sp_debug("MPROTECT - Failed to change memory access permission");
    }

    sp_debug("AFTER INSTALL CALL BLK (%lu bytes) for point %lx - {",
             callblk->size(), callblk->last());
    sp_debug("%s", g_parser->DumpInsns((void*)callblk->start(),
                                     callblk->size()).c_str());
    sp_debug("}");

    sp_debug("AFTER INSTALL SPRING BLK (%lu bytes) for point %lx - {",
             springblk->size(), callblk->last());
    sp_debug("%s", g_parser->DumpInsns((void*)springblk->start(),
                                     springblk->size()).c_str());
    sp_debug("}");

    sp_debug("USE SPRING - piont %lx is instrumented using 1-hop spring",
             callblk->last());

    return true;
  }

	size_t
	SpringboardWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocBlockSize(pt);
	}

}
