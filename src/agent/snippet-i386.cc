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

#include "agent/context.h"
#include "agent/event.h"
#include "agent/parser.h"
#include "agent/patchapi/point.h"
#include "agent/snippet.h"
#include "common/utils.h"


namespace sp {
  extern SpContext* g_context;

  // -------------------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------------------

  // Save context before calling payload
  size_t
  SpSnippet::emit_save(char* buf,
                       size_t offset) {
    char* p = buf + offset;
    // pusha
    *p++ = 0x60;
    size_t insnsize = emit_save_sp(p, 0);
    p += insnsize;
    return (p - (buf + offset));
  }

  // Restore context after calling payload
  size_t
  SpSnippet::emit_restore(char* buf,
                          size_t offset) {
    char* p = buf + offset;
    // popa
    *p++ = 0x61;
    return (p - (buf + offset));
  }

  // For debugging, cause segment fault
  size_t
  SpSnippet::emit_fault(char* buf,
                        size_t offset) {
    char* p = buf + offset;

    // mov $0, 0
    *p++ = (char)0xc7;
    *p++ = (char)0x05;

    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;

    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;

    return (p - (buf + offset));
  }

  // Save stack pionter, for two purposes
  // 1. Resolve indirect call during runtime
  // 2. Get argument of callees in payload function
  size_t
  SpSnippet::emit_save_sp(char* buf,
                          size_t offset) {
    char* p = buf + offset;

    *p++ = 0x89;   // mov %esp, (loc)
    *p++ = 0x25;

    long* l = (long*)p;
    *l = (long)&saved_context_loc_;
    p += sizeof(long);

    return (p - (buf + offset));
  }

  // Push an imm32 in stack
  static size_t
  emit_push_imm32(long imm,
                  char* buf,
                  size_t offset) {
    char* p = buf + offset;

    // push imm32
    *p++ = 0x68;
    long* i = (long*)p;
    *i = imm;
    p += sizeof(long);

    return (p - (buf + offset));
  }

  // Get the saved imm32 from stack to %eax
  static size_t
  emit_pop_imm32(char* buf,
                 size_t offset) {
    char* p = buf + offset;
    // pop %eax
    *p++ = 0x58;
    return (p - (buf + offset));
  }

  // Pass parameter to payload function
  // Two parameters - POINT and Payload function
  // If payload == 0, then we are dealing with single-process only
  size_t
  SpSnippet::emit_pass_param(long point,
                             long payload,
                             char* buf,
                             size_t offset) {
    char* p = buf + offset;
    size_t insnsize = 0;

    if (payload) {
      // push payload
      insnsize = emit_push_imm32((long)payload, p, 0);
      p += insnsize;
    }

    // push POINT
    insnsize = emit_push_imm32((long)point, p, 0);
    p += insnsize;

    return (p - (buf + offset));
  }

  // Call a function w/ address `callee`
  // `restore` indicates whether we need to pop out the pushed argument,
  // this only happens when we call payload function
  size_t
  SpSnippet::emit_call_abs(long callee,
                           char* buf,
                           size_t offset,
                           bool restore) {
    char* p = buf + offset;
    dt::Address retaddr = (dt::Address)p+5;
    size_t insnsize = 0;
    dt::Address rel_addr = (callee - retaddr);

    if (sp::IsDisp32(rel_addr)) {
      // call callee
      *p++ = 0xe8;
      int* rel_p = (int*)p;
      *rel_p = rel_addr;
      p += sizeof(long);
    } else {
      sp_perror("larger than 4 bytes for call address");
    }

    if (restore) {
      // pop point
      insnsize = emit_pop_imm32(p, 0);
      p += insnsize;

      if (g_context->IsIpcEnabled()) {
        // pop payload
        insnsize = emit_pop_imm32(p, 0);
        p += insnsize;
      }
    }

    return (p - (buf + offset));
  }

  // Jump to `trg`.
  // `abs` indicates whether we must use absolute jump. If `abs` is false,
  // we make such decision dynamically; otherwise, we mandate absolute
  // jump for determinism.
  size_t
  SpSnippet::emit_jump_abs(long trg,
                           char* buf,
                           size_t offset,
                           bool abs) {
    char* p = buf + offset;
    size_t insnsize = 0;

    dt::Address retaddr = (dt::Address)p+5;
    dt::Address rel_addr = (trg - retaddr);

    if (sp::IsDisp32(rel_addr) && !abs) {
      // jmp trg
      *p++ = 0xe9;
      int* rel_p = (int*)p;
      *rel_p = rel_addr;
      p += 4;
    } else {
      // push trg
      insnsize = emit_push_imm32(trg, p, 0);
      p += insnsize;
      // ret
      *p++ = 0xc3;
    }

    return (p - (buf + offset));
  }

  // Relocate the call instruction
  // This is used in dealing with indirect call
  size_t
  SpSnippet::emit_call_orig(char* buf,
                            size_t offset) {
    SpBlock* b = point_->GetBlock();
    assert(b);

    char* p = buf + offset;
    char* psrc = (char*)b->orig_call_insn()->ptr();
    size_t size = b->orig_call_insn()->size();
    memcpy(p, psrc, size);
    return size;
  }

  // Relocate an ordinary instruction
  size_t
  SpSnippet::reloc_insn(dt::Address src_insn,
                        in::Instruction::Ptr insn,
                        dt::Address last,
                        char* buf) {
    // Case 1: we don't relocate the last insn
    // because we'll do it later
    dt::Address a = src_insn;
    if (a == last)  return 0;

    char* p = buf;
    if (insn->getCategory() == in::c_CallInsn &&
        a != last)  {
      sp_debug("THUNK CALL - at insn %lx", src_insn);
      // Case 2: handle thunk call
      // What thunk does, is to move current pc value to ebx.
      // mov orig_pc, ebx
      *p++ = 0xbb;
      long* new_ebx = (long*)p;
      *new_ebx = (long)(a+5);
      return 5;
    } else {
      // Case 3: other instructions
      memcpy(p, insn->ptr(), insn->size());
      return insn->size();
    }
  }

  // -------------------------------------------------------------------
  // Miscellaneous
  // -------------------------------------------------------------------

  // Used in trap handler to decide the pc value right at the call
  dt::Address
  SpSnippet::get_pre_signal_pc(void* context) {
    ucontext_t* ctx = (ucontext_t*)context;
    return ctx->uc_mcontext.gregs[REG_EIP];
  }

  // Used in trap handler to jump to snippet
  dt::Address
  SpSnippet::set_pc(dt::Address pc,
                    void* context) {
    ucontext_t* ctx = (ucontext_t*)context;
    ctx->uc_mcontext.gregs[REG_EIP] = pc;
    return pc;
  }

  // Get the saved register, for resolving indirect call
  dt::Address
  SpSnippet::GetSavedReg(Dyninst::MachRegister reg) {

    // Pushed Left to right in order:
    // Push(EAX); Push(ECX); Push(EDX); Push(EBX); Push(Temp); Push(EBP); Push(ESI); Push(EDI);
#define EDI  0
#define ESI  4
#define EBP  8
#define ESP  12
#define EBX  16
#define EDX  20
#define ECX  24
#define EAX  28

    /*
      for (int i = 0; i < 32; i+=4) {
      sp_debug("i: %d, EDI: %lx", i, reg_val(i));
      }
    */
    namespace d32 = Dyninst::x86;

    dt::Address out = 0;
    if (GetRegInternal(reg,
                       &out)) {
      sp_debug("GOT REG - %s = %lx", reg.name().c_str(), out);
      return out;
    }

    sp_debug("NOT FOUND - %s", reg.name().c_str());
    return 0;
  }


  // The upper bound for a jump instruction.
  size_t
  SpSnippet::jump_abs_size() {
    // pushl imm;
    // ret
    return 7;
  }

  // Get argument of a function call
  void*
  SpSnippet::pop_argument(ArgumentHandle* h,
                          size_t size) {
    void* a = (void*)(saved_context_loc_ + 32 + h->offset);
    h->offset += size;
    return a;
  }

  // Get return value of a function call
  long
  SpSnippet::get_ret_val() {
    return GetSavedReg(Dyninst::x86::eax);
  }

  void
  SpSnippet::InitSavedRegMap() {
    namespace d32 = Dyninst::x86;

    saved_reg_map_[d32::eax] = EAX;
    saved_reg_map_[d32::ah] = EAX;
    saved_reg_map_[d32::al] = EAX;

    saved_reg_map_[d32::ebx] = EBX;
    saved_reg_map_[d32::bh] = EBX;
    saved_reg_map_[d32::bl] = EBX;

    saved_reg_map_[d32::ecx] = ECX;
    saved_reg_map_[d32::ch] = ECX;
    saved_reg_map_[d32::cl] = ECX;

    saved_reg_map_[d32::edx] = EDX;
    saved_reg_map_[d32::dh] = EDX;
    saved_reg_map_[d32::dl] = EDX;

    saved_reg_map_[d32::ebp] = EBP;
    saved_reg_map_[d32::bp] = EBP;

    saved_reg_map_[d32::esp] = ESP;
    saved_reg_map_[d32::sp] = ESP;

    saved_reg_map_[d32::esi] = ESI;
    saved_reg_map_[d32::si] = ESI;

    saved_reg_map_[d32::edi] = EDI;
    saved_reg_map_[d32::di] = EDI;
  }
}
