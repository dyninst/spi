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

// Snippet to hold relocated code and the invocation of payload function.


#ifndef _SPSNIPPET_H_
#define _SPSNIPPET_H_

#include "common/common.h"
#include "agent/payload.h"

namespace sp {

  class SpFunction;


  class SpSnippet {
    friend class SpInstrumenter;
    friend class RelocCallBlockWorker;
    friend class SpringboardWorker;
  public:
    typedef SHARED_PTR(SpSnippet) ptr;
    static ptr create(SpFunction* f,
                      SpPoint* pt,
                      PayloadFunc entry,
                      PayloadFunc exit) {
      return ptr(new SpSnippet(f, pt, entry, exit));
    }
    SpSnippet(SpFunction* f,
              SpPoint* pt,
              PayloadFunc entry,
              PayloadFunc exit);
    ~SpSnippet();

    // Return the pointer to blob, but blob is empty
    // If estimate_size is non-zero, we (re)allocate blob
    dt::Address GetBlob(const size_t estimate_size = 0);

    // Return the pointer to blob, and fill the blob
    char* BuildBlob(const size_t est_size,
                    const bool reloc = false);
    size_t GetBlobSize() const { return blob_size_; }

    // Springboard stuffs
    SpBlock* FindSpringboard();
    char* RelocateSpring(SpBlock* spring_blk);
    size_t GetRelocSpringSize() const {return spring_size_; }

    dt::Address GetSavedReg(dt::MachRegister reg);
    long GetRetVal();
    void* PopArgument(ArgumentHandle* h, size_t size);

    // Some getters
    PayloadFunc entry() const { return entry_; }
    PayloadFunc exit() const { return exit_; }
    SpPoint* point() const { return point_; }
    SpFunction* func() const { return func_; }

    // Find and return a spring block; if not found, return NULL
    static dt::Address get_pre_signal_pc(void* context);
    static dt::Address GetFs(void* context);
    static dt::Address set_pc(dt::Address pc, void* context);
    static size_t jump_abs_size();

    static bool UsePC(in::Instruction::Ptr);
    
  protected:
    SpFunction* func_;
    SpPoint* point_;
    PayloadFunc entry_;
    PayloadFunc exit_;
    long saved_context_loc_;

    // Blob things
    char* blob_;
    size_t blob_size_;

    // Spring block things
    char* spring_;
    size_t spring_size_;
    SpBlock* spring_blk_;

    // Handle saved registers
    typedef std::map<dt::MachRegister, int> SavedRegMap;
    SavedRegMap saved_reg_map_;

    void InitSavedRegMap();

    inline dt::Address
    RegVal(long offset) {
      return *(long*)(saved_context_loc_ + offset);
    }

    bool
    GetRegInternal(dt::MachRegister reg,
                   dt::Address* out);

    // A bunch of code generation interfaces
    size_t emit_save(char* buf, size_t offset);
    size_t emit_restore( char* buf, size_t offset);
    size_t emit_fault(char* buf, size_t offset);
    size_t emit_pass_param(long point, long payload, char* buf, size_t offset);
    size_t emit_call_abs(long callee, char* buf, size_t offset, bool restore);
    size_t emit_call_orig(char* buf, size_t offset);
    size_t emit_save_sp(char* buf, size_t offset);
    size_t emit_jump_abs(long trg, char* buf, size_t offset, bool abs = false);

    // relocate
    size_t reloc_block(SpBlock* blk,
                       char* buf,
                       size_t offset);
    size_t reloc_insn(dt::Address src_insn,
                      in::Instruction::Ptr insn,
                      dt::Address last,
                      char* buf);

  };

}

#endif /* _SPSNIPPET_H_ */
