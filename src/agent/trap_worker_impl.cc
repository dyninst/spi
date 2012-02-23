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

#include "agent/object.h"
#include "agent/parser.h"
#include "agent/context.h"
#include "agent/addr_space.h"
#include "agent/trap_worker_impl.h"

namespace sp {
  extern SpContext* g_context;
	extern SpAddrSpace* g_as;
	extern SpParser::ptr g_parser;

	// Used in trap handler, for mapping call instruction's address to snippet
  TrapWorker::InstMap TrapWorker::inst_map_;

  bool
  TrapWorker::run(SpPoint* pt) {
    sp_debug("TRAP WORKER - runs");
    assert(pt);

    // Install trap handler to the trap signal
    struct sigaction act;
    act.sa_sigaction = TrapWorker::OnTrap;
    act.sa_flags = SA_SIGINFO;
    struct sigaction old_act;
    sigaction(SIGTRAP, &act, &old_act);

    // Call insn's addr
		assert(pt->GetBlock());
		dt::Address call_addr = pt->GetBlock()->last();

    // This mapping is used in trap handler
		assert(pt->snip());
    inst_map_[call_addr] = pt->snip();

    // Install
    return install(pt);
  }

  bool
  TrapWorker::undo(SpPoint* pt) {
    return true;
  }

  bool
  TrapWorker::install(SpPoint* pt) {
		assert(pt);
		assert(pt->snip());

		SpBlock* b = pt->GetBlock();
		assert(b);
		
    sp_debug("TRAP WORKER - installs");

    sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)b->size(),
						 (unsigned long)b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->size()).c_str());
    sp_debug("}");

    char* call_addr = (char*)b->last();
		assert(call_addr);

		size_t est_size = EstimateBlobSize(pt);
		char* blob = pt->snip()->BuildBlob(est_size);
    if (!blob) {
			sp_debug("FAILED BLOB - failed to generate blob for call insn %lx",
							 (unsigned long)call_addr);
			return false;
		}

    char int3 = (char)0xcc;
    size_t call_size = b->call_size();

    // Overwrite int3 to the call site
    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		SpObject* obj = pt->GetObject();
		assert(obj);
		assert(g_as);

    if (!g_as->SetCodePermission((dt::Address)call_addr, call_size, perm)) {
      sp_debug("FAILED PERM - failed to change memory permission");
      return false;
    } else {
      g_as->write(obj, (dt::Address)call_addr, (dt::Address)&int3, 1);
    }

    // Restore the permission of memory mapping
    if (!g_as->RestoreCodePermission((dt::Address)call_addr, call_size)) {
			sp_debug("FAILED RESTORE - failed to restore perm for call insn %lx",
							 (unsigned long)call_addr);
      return false;
    }

    sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
             (unsigned long)b->size(),
						 (unsigned long)b->last());
    sp_debug("%s", g_parser->dump_insn((void*)b->start(),
																			 b->last() - b->start() +1).c_str());
    sp_debug("}");

    sp_debug("TRAP INSTALLED - successful for call insn %lx",
						 (long)call_addr);
    return true;
  }

  // We resort to trap to transfer control to patch area, when we are not
  // able to use jump-based implementation.
  void
  TrapWorker::OnTrap(int sig, siginfo_t* info, void* c) {
    dt::Address pc = SpSnippet::get_pre_signal_pc(c) - 1;
		assert(pc);

    InstMap& inst_map = TrapWorker::inst_map_;
    if (inst_map.find(pc) == inst_map.end()) {
      sp_perror("NO BLOB - for this call insn at %lx", pc);
    }

    // Get patch area's address
    SpSnippet::ptr sp_snip = inst_map[pc];
		assert(sp_snip);

    char* blob = (char*)sp_snip->GetBlob();
		assert(sp_snip);

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
		sp_debug("TRAP HANDLER - for call insn %lx", pc);

		assert(g_as);
    // Change memory permission for the snippet
    if (!g_as->SetSnippetPermission((dt::Address)blob,
                                    sp_snip->size(), perm)) {
      // g_as->dump_mem_maps();
      sp_perror("FAILED PERM - failed to change memory permission for blob");
    }

		sp_debug("JUMP TO BLOB - at %lx", (dt::Address)blob);

    // Set pc to jump to patch area
    SpSnippet::set_pc((dt::Address)blob, c);
  }
}
