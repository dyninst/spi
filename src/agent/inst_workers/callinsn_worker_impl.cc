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
#include "agent/inst_workers/callinsn_worker_impl.h"
#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"

namespace sp {
  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

  bool
  RelocCallInsnWorker::run(SpPoint* pt) {
    sp_debug("RELOC CALLINSN WORKER - runs");

		assert(pt);
		SpBlock* b = pt->GetBlock();
		assert(b);

    // Check if we are able to overwrite the call insn w/ a short jmp,
    // where we check two things:

    dt::Address call_insn_addr = b->last();
    char* call_insn = (char*)call_insn_addr;
		assert(call_insn);

		size_t call_insn_size = b->call_size();

		// 1. is the instruction >= 5 bytes?
		if (call_insn_size < 5) {
			sp_debug("SMALL CALL INSN - call insn is too small (< 5-byte),"
							 " try other workers");
			return false;
		}

    // 2. is it a direct call instruction
    if (call_insn[0] != (char)0xe8) {  // A direct call?
			sp_debug("NOT NORMAL CALL");
			if (!pt->tailcall()) {           // A jump for direct tail call?
				sp_debug("NOT TAIL CALL - try other workers");
				return false;
			} else {
				sp_debug("IS TAIL CALL");
				assert(call_insn_size == 5 &&
							 "LARGE INDIRECT TAIL CALL, UNSUPPORTED");
			}
    }

    // 3. is the relative address to snippet within 4-byte?
		SpSnippet::ptr snip = pt->snip();
		assert(snip);
		size_t est_size = EstimateBlobSize(pt);
		dt::Address blob = snip->GetBlob(est_size);
		assert(blob);
    long rel_addr = (long)blob - (long)call_insn_addr;
    if (!sp::IsDisp32(rel_addr)) {
      sp_debug("NOT 4-byte DISP - try other workers");
      return false;
    }

    return install(pt);
  }

  bool
  RelocCallInsnWorker::undo(SpPoint* pt) {
    assert(0 && "TODO");
    return true;
  }

  bool
  RelocCallInsnWorker::install(SpPoint* pt) {
		assert(pt);
		SpBlock* b = pt->GetBlock();
		assert(b);

    // Generate the snippet
    // XXX: should we move it to the SpInstrumenter::run() ?
    char* call_addr = (char*)b->last();
		assert(call_addr);

		SpSnippet::ptr snip = pt->snip();
		assert(snip);

		size_t est_size = EstimateBlobSize(pt);
		char* blob = snip->BuildBlob(est_size);
		assert(blob);
    size_t blob_size = snip->size();
    if (!blob) {
			sp_debug("FAILED TO GENERATE BLOB");
			return false;
		}

    char jump[5];
    memset(jump, 0, 5);
    char* p = jump;
    *p++ = 0xe9;
    // For relative address to snippet
    int* lp = (int*)p;

		assert(pt->callee());
    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx for %s- {",
             (unsigned long)b->size(), (unsigned long)call_addr,
             pt->callee()->name().c_str());
    sp_debug("%s", g_parser->DumpInsns((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    // Build the jump instruction
		SpObject* obj = pt->GetObject();
		assert(obj);

    size_t insn_length = b->call_size();
    *lp = (int)((long)blob - (long)call_addr - insn_length);

    sp_debug("REL CAL - blob at %lx, last insn at %lx, insn length %lu",
             (unsigned long)blob, b->end(),
						 (unsigned long)insn_length);
    sp_debug("JUMP REL - jump to relative address %x", *lp);

    // Replace "call" with a "jump" instruction

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		assert(g_as);
    if (g_as->SetMemoryPermission((dt::Address)call_addr, insn_length, perm)) {
      g_as->write(obj, (dt::Address)call_addr, (dt::Address)jump, 5);
    } else {
      sp_print("MPROTECT - Failed to change memory access permission");
    }

    // Change the permission of snippet, so that it can be executed.
    if (!g_as->SetMemoryPermission((dt::Address)blob, blob_size, perm)) {
      sp_print("MPROTECT - Failed to change memory access permission"
               " for blob at %lx", (dt::Address)blob);
      // g_as->dump_mem_maps();
      exit(0);
    }
    
		assert(pt->callee());

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx for %s- {",
             b->size(), b->last(),
             pt->callee()->name().c_str());
    sp_debug("%s", g_parser->DumpInsns((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    return true;
  }

	size_t
	RelocCallInsnWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
	}

}
