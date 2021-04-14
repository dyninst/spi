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
#include "agent/inst_workers/trap_worker_impl.h"
#include "agent/patchapi/addr_space.h"
#include "agent/parser.h"
#include "agent/patchapi/object.h"

#include "frame.h"
#include "walker.h"

#include <signal.h>
namespace sp {
extern SpContext* g_context;
extern SpAddrSpace* g_as;
extern SpParser::ptr g_parser;

// Used in trap handler, for mapping call instruction's address to snippet
TrapWorker::InstMap TrapWorker::inst_map_;

bool TrapWorker::run(SpPoint* pt) {
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
  sp_debug("snippet added to instrumentation map at %xl", call_addr);

  // Install
  return install(pt);
}

bool TrapWorker::undo(SpPoint* pt) { return true; }

bool TrapWorker::install(SpPoint* pt) {
  assert(pt);
  assert(pt->snip());

  SpBlock* b = pt->GetBlock();
  assert(b);

  sp_debug("TRAP WORKER - installs");

  sp_debug("BEFORE INSTALL (%lu bytes) for point %lx - {",
           (unsigned long)b->size(), (unsigned long)b->last());
  sp_debug("%s", g_parser->DumpInsns((void*)b->start(), b->size()).c_str());
  sp_debug("}");

  char* call_addr = (char*)b->last();
  assert(call_addr);
  if (!call_addr || (long)call_addr < getpagesize()) {
    return false;
  }

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

  if (!g_as->SetMemoryPermission((dt::Address)call_addr, call_size, perm)) {
    sp_debug("FAILED PERM - failed to change memory permission");
    return false;
  } else {
    g_as->write(obj, (dt::Address)call_addr, (dt::Address)&int3, 1);
  }

  sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
           (unsigned long)b->size(), (unsigned long)b->last());
  sp_debug("%s",
           g_parser->DumpInsns((void*)b->start(), b->last() - b->start() + 1)
               .c_str());
  sp_debug("}");

  sp_debug("TRAP INSTALLED - successful for call insn %lx", (long)call_addr);
  return true;
}

// Replace all return instructions with a trap instruction to
// go to the exit point instrumentation
bool TrapWorker::ReplaceReturnWithTrap(SpPoint* pt) {
  struct sigaction act;
  act.sa_sigaction = TrapWorker::OnTrap;
  act.sa_flags = SA_SIGINFO;
  struct sigaction old_act;
  sigaction(SIGTRAP, &act, &old_act);

  SpBlock* b = pt->GetBlock();
  sp_debug("BEFORE INSTALL (%lu bytes) for return point %lx - {",
           (unsigned long)b->size(), (unsigned long)b->last());
  sp_debug("%s", g_parser->DumpInsns((void*)b->start(), b->size()).c_str());
  sp_debug("}");

  char* ret_addr = (char*)b->last();
  assert(ret_addr);
  if (!ret_addr || (long)ret_addr < getpagesize()) {
    return false;
  }

  char int3 = (char)0xcc;

  // Overwrite int3 to the return point
  int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
  SpObject* obj = pt->GetObject();
  assert(obj);
  assert(g_as);

  if (!g_as->SetMemoryPermission((dt::Address)ret_addr, 1, perm)) {
    sp_debug("FAILED PERM - failed to change memory permission");
    return false;
  } else {
    g_as->write(obj, (dt::Address)ret_addr, (dt::Address)&int3, 1);
  }

  sp_debug("AFTER INSTALL (%lu bytes) for point %lx - {",
           (unsigned long)b->size(), (unsigned long)b->last());
  sp_debug("%s",
           g_parser->DumpInsns((void*)b->start(), b->last() - b->start() + 1)
               .c_str());
  sp_debug("}");

  sp_debug("TRAP INSTALLED - successful for return insn %lx", (long)ret_addr);
  return true;
}

// We resort to trap to transfer control to patch area, when we are not
// able to use jump-based implementation.
void TrapWorker::OnTrap(int sig, siginfo_t* info, void* c) {
  dt::Address pc = SpSnippet::get_pre_signal_pc(c) - 1;
  assert(pc);
  InstMap& inst_map = TrapWorker::inst_map_;
  if (inst_map.find(pc) == inst_map.end()) {
    sp_debug("instrumentation map for %lx not found", pc);
    dt::Address ret = g_context->GetReturnAddress();

    if (ret != (dt::Address)0) {
      sp_debug("TRAP HANDLER - for ret insn %lx", ret);

      // Find the call site point corresponding to the return address
      SpPoint* call_site_point = g_context->FindCallSitePointFromRetAddr(ret);
      if (!call_site_point)
        sp_perror(
            "A call site point has not been registered for the return address "
            "%lx",
            ret);
      sp_debug("Got point %lx", (dt::Address)call_site_point);

      // Find the exit instrumentation address correspoding to the call point
      char* exit_snip_addr =
          g_context->FindExitInstAddrFromCallSitePoint(call_site_point);
      if (!exit_snip_addr)
        sp_perror(
            "Exit Instrumentation address cannot be found for the call site "
            "point %lx",
            (dt::Address)call_site_point);
      sp_debug("Got exit instrumentation address %lx",
               (dt::Address)exit_snip_addr);

      // The x86-64 needs its stack to be 16 byte aligned for accessing XMM
      // registers
      dt::Address esp = SpSnippet::align_stack(c);
      sp_debug("Stack pointer aligned at %lx", esp);

      // sp_debug("%s", g_parser->DumpInsns((void*)exit_snip_addr,150).c_str());
      sp_debug("Jump to exit point instrumentation at %lx ",
               (dt::Address)exit_snip_addr);

      // Set pc to jump to the exit point instrumentation
      SpSnippet::set_pc((dt::Address)exit_snip_addr, c);
    } else {
      sp_debug("ret 0 inside trap handler");
    }
  } else {
    // Get patch area's address
    SpSnippet::ptr sp_snip = inst_map[pc];
    assert(sp_snip);
    char* blob = (char*)sp_snip->GetBlob();
    if (!blob || (long)blob < getpagesize()) {
      sp_debug("TRAP invalid BLOB - at %lx, blob is %lx", pc,
               (dt::Address)blob);
      return;
    }

    int perm = PROT_READ | PROT_WRITE | PROT_EXEC;
    sp_debug("TRAP HANDLER - for call insn %lx", pc);

    assert(g_as);
    // Change memory permission for the snippet
    if (!g_as->SetMemoryPermission((dt::Address)blob, sp_snip->GetBlobSize(),
                                   perm)) {
      // g_as->dump_mem_maps();
      sp_perror("FAILED PERM - failed to change memory permission for blob");
    }

    sp_debug("JUMP TO BLOB - at %s Address %lx", blob, (dt::Address)blob);
    // Set pc to jump to patch area
    SpSnippet::set_pc((dt::Address)blob, c);
  }
}

size_t TrapWorker::EstimateBlobSize(SpPoint* pt) {
  return InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
}

}  // namespace sp
