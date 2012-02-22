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

#include "agent/addr_space.h"
#include "agent/context.h"
#include "agent/object.h"
#include "agent/parser.h"
#include "agent/callblk_worker_impl.h"

namespace sp {
  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;


  bool
  RelocCallBlockWorker::run(SpPoint* pt) {
    sp_debug("RELOC CALLBLOCK WORKER - runs");

    return install(pt);
  }

  bool
  RelocCallBlockWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }


  bool
  RelocCallBlockWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    dt::Address call_blk_addr = b->start();

    // Try to install short jump
		SpSnippet::ptr snip = pt->snip();
		assert(snip);
		size_t est_size = est_blob_size(pt);
		dt::Address blob = snip->get_blob(est_size);
		assert(blob);
    long rel_addr = (long)blob - (long)call_blk_addr - 5;
    char insn[64];    // the jump instruction to overwrite call blk

    if (sp::IsDisp32(rel_addr)) {
      sp_debug("4-byte DISP - install a short jump");

      // Generate a short jump to store in insn[64]
      char* p = insn;
      *p++ = 0xe9;
      long* lp = (long*)p;
      *lp = rel_addr;

      return install_jump_to_block(pt, insn, 5);
    }

    // Try to install long jump
    if (b->size() >= snip->jump_abs_size()) {

      sp_debug("> 4-byte DISP - install a long jump");

      // Generate a long jump to store in insn[64]
      size_t insn_size = snip->emit_jump_abs((long)blob,
																						 insn, 0, true);

      return install_jump_to_block(pt, insn, insn_size);
    } else {
      sp_debug("CALL BLK TOO SMALL - %ld < %ld", b->size(),
							 snip->jump_abs_size());
		}

    // Well, let's try spring board next ...
    sp_debug("FAILED RELOC BLK - try other worker");

    return false;
  }

  bool RelocCallBlockWorker::install_jump_to_block(SpPoint* pt,
                                                   char* jump_insn,
                                                   size_t insn_size) {

		assert(pt);
		SpBlock* b = pt->get_block();
		assert(b);

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             b->size(), b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

		SpSnippet::ptr snip = pt->snip();
		assert(snip);

    // Build blob & change the permission of snippet
		size_t est_size = est_blob_size(pt);
    char* blob = snip->build_blob(est_size,
																	/*reloc=*/true);
		if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}

		SpObject* obj = pt->get_object();
		assert(obj);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (!g_as->set_range_perm((dt::Address)blob, snip->size(), perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }

    char* addr = (char*)b->start();
		assert(addr);

    // Write a jump instruction to call block
    if (g_as->set_range_perm((dt::Address)addr, insn_size, perm)) {
      g_as->write(obj, (dt::Address)addr, (dt::Address)jump_insn, insn_size);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Restore the permission of memory mapping
    if (!g_as->restore_range_perm((dt::Address)addr, insn_size)) {
      sp_print("MPROTECT - Failed to restore memory access permission");
    }

    sp_debug("USE BLK-RELOC - piont %lx is instrumented using call"
             " block relocation", b->last());

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             b->size(), b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    return true;
  }


}
