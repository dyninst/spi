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
//#define arch_x86_64
//#define os_osf

#include <stack>

#include "common/common.h"

#include "agent/context.h"
#include "agent/event.h"
#include "agent/parser.h"
#include "agent/patchapi/instrumenter.h"
#include "agent/patchapi/point.h"
#include "agent/snippet.h"

#include "common/utils.h"

#include "Visitor.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "dyn_regs.h"
#include "Instruction.h"
#include "arch-x86.h"

using dt::Address;
using namespace NS_x86;

namespace sp {
extern SpContext* g_context;
extern SpParser::ptr g_parser;

// Code Generation stuffs

// Save context before calling payload
size_t SpSnippet::emit_save(char* buf, size_t offset, bool save_fp_regs) {
  assert(buf);
  char* p = buf + offset;

  bool indirect = false;
  if (!func_) {
    indirect = true;
  }

  // Saved for direct/indirect call
  *p++ = 0x57;  // push rdi
  *p++ = 0x56;  // push rsi
  *p++ = 0x52;  // push rdx
  *p++ = 0x51;  // push rcx
  *p++ = 0x41;  // push r8
  *p++ = 0x50;
  *p++ = 0x41;  // push r9
  *p++ = 0x51;

  *p++ = 0x50;  // push rax

  // Store flags
  *p++ = 0x9f;  // lahf # save the eflags byte into %ah
  *p++ = 0x0f;
  *p++ = 0x90;
  *p++ = 0xc0;  // seto %al  #save the overflow flag into %al

  *p++ = 0x50;  // push rax again to store the eflags also, this makes the stack
                // 16 byte aligned, for saving floating point registers

  // Save stack pointer at this point, for future lookup for other
  // saved registers at runtime.
  p += emit_save_sp(p, 0);

  // Saved for indirect call only
  if (indirect) {
    *p++ = 0x41;  // push r10 -- unused in C
    *p++ = 0x52;
    *p++ = 0x41;  // push r11 -- for linker
    *p++ = 0x53;
    *p++ = 0x53;  // push rbx
    *p++ = 0x41;  // push r12
    *p++ = 0x54;
    *p++ = 0x41;  // push r13
    *p++ = 0x55;
    *p++ = 0x41;  // push r14
    *p++ = 0x56;
    *p++ = 0x41;  // push r15
    *p++ = 0x57;
    *p++ = 0x55;  // push rbp
  }
  // Save floating point registers
  if (save_fp_regs) {
    // *p++ = 0x0f;
    // *p++ = 0xae;
    // *p++ = 0x04;
    // *p++ = 0x24;
    p += emit_save_fp_registers(p, 0);
  }

  return (p - (buf + offset));
}

// Save floating point registers
size_t SpSnippet::emit_save_fp_registers(char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;

  // Idea is to shift the stack pointer down(by 136 bytes, then
  // copy RSP to RAX, then use RAX+8 as an index to save the
  // floating point registers.
  bool isTailCall = point()->tailcall();
  if (!isTailCall) {
    *p++ = 0x48;  // lea -0x88(%rsp),%rsp - with rex prefix
    *p++ = 0x8d;
    *p++ = 0xa4;
    *p++ = 0x24;
    *p++ = 0x78;
    *p++ = 0xff;
    *p++ = 0xff;
    *p++ = 0xff;
  } else {
    *p++ = 0x48;  // lea -0x88(%rsp),%rsp - with rex prefix
    *p++ = 0x8d;
    *p++ = 0xa4;
    *p++ = 0x24;
    *p++ = 0x70;
    *p++ = 0xff;
    *p++ = 0xff;
    *p++ = 0xff;
  }

  *p++ = 0x48;  // mov %rsp,%rax
  *p++ = 0x8b;
  *p++ = 0xc4;

  *p++ = 0x48;  // add %RAX,0x8
  *p++ = 0x05;
  *p++ = 0x08;
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;

  // Now save all floating point regs
  p += emitXMMRegsSaveRestore(p, 0, false);

  // hack: Align the stack pointer again(simply use push of any register)
  *p++ = 0x50;

  return (p - (buf + offset));
}
// This function code is borrowed from Dyninst
size_t SpSnippet::emitXMMRegsSaveRestore(char* buf, size_t off,
                                         bool isRestore) {
  // Quick hack time: save at +0 to +0x70 (so occupying 0..0x80)
  // and 0..7
  assert(buf);
  char* p = buf + off;
  for (unsigned xmm_reg = 0; xmm_reg <= 7; ++xmm_reg) {
    unsigned char offset = xmm_reg * 16;
    *p++ = 0x66;
    *p++ = 0x0f;
    if (isRestore)
      *p++ = 0x6f;
    else
      *p++ = 0x7f;

    if (xmm_reg == 0) {
      *p++ = 0x00;
    } else {
      unsigned char modrm = 0x40 + (0x8 * xmm_reg);
      *p++ = modrm;
      *p++ = offset;
    }
  }
  return (p - (buf + off));
}

// Restore context after calling payload
size_t SpSnippet::emit_restore(char* buf, size_t offset, bool restore_fp_regs) {
  assert(buf);
  char* p = buf + offset;

  bool indirect = false;
  if (!func_) {
    indirect = true;
  }

  // Restore floating point regsiters
  if (restore_fp_regs) {
    p += emit_restore_fp_registers(p, 0);
  }
  // Restored for indirect call
  if (indirect) {
    *p++ = 0x5d;  // pop rbp
    *p++ = 0x41;  // pop r15
    *p++ = 0x5f;
    *p++ = 0x41;  // pop r14
    *p++ = 0x5e;
    *p++ = 0x41;  // pop r13
    *p++ = 0x5d;
    *p++ = 0x41;  // pop r12
    *p++ = 0x5c;
    *p++ = 0x5b;  // pop rbx
    *p++ = 0x41;  // pop r11
    *p++ = 0x5b;
    *p++ = 0x41;  // pop r10
    *p++ = 0x5a;
  }

  // Restored for direct/indirect call
  *p++ = 0x58;  // pop rax (to make the stack 16 byte aligned)

  // Restore flags
  *p++ = 0x80;
  *p++ = 0xc0;
  *p++ = 0x7f;  // add $0x7f, %al  #restore overflow flag from %al
  *p++ = 0x9e;  // sahf  # restore %eflags byte from %ah

  *p++ = 0x58;  // pop rax
  *p++ = 0x41;  // pop r9
  *p++ = 0x59;
  *p++ = 0x41;  // pop r8
  *p++ = 0x58;
  *p++ = 0x59;  // pop rcx
  *p++ = 0x5a;  // pop rdx
  *p++ = 0x5e;  // pop rsi
  *p++ = 0x5f;  // pop rdi

  return (p - (buf + offset));
}

// Restore floating point registers
size_t SpSnippet::emit_restore_fp_registers(char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;

  // hack: Align the stack pointer again(pop the register which is pushed)
  *p++ = 0x58;

  *p++ = 0x48;  // mov %rsp, %rax
  *p++ = 0x8b;
  *p++ = 0xc4;

  *p++ = 0x48;  // add %RAX,0x8
  *p++ = 0x05;
  *p++ = 0x08;
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;

  p += emitXMMRegsSaveRestore(p, 0, true);  // restore fp registers

  bool isTailCall = point()->tailcall();
  if (!isTailCall) {
    *p++ = 0x48;  // lea 0x88(%rsp), %rsp
    *p++ = 0x8d;
    *p++ = 0xa4;
    *p++ = 0x24;
    *p++ = 0x88;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
  } else {
    *p++ = 0x48;  // lea 0x88(%rsp), %rsp
    *p++ = 0x8d;
    *p++ = 0xa4;
    *p++ = 0x24;
    *p++ = 0x90;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
  }

  return (p - (buf + offset));
}

// Save stack pionter, for two purposes
// 1. Resolve indirect call during runtime
// 2. Get argument of callees in payload function
size_t SpSnippet::emit_save_sp(char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;

  // mov loc, %rax
  *p++ = 0x48;
  *p++ = 0xb8;
  long* l = (long*)p;

  *l = (long)&saved_context_loc_;
  p += sizeof(long);

  // mov %rsp, (%rax)
  *p++ = 0x48;
  *p++ = 0x89;
  *p++ = 0x20;

  return (p - (buf + offset));
}

// For debugging, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;

  // mov $0, 0
  *p++ = (char)0x48;
  *p++ = (char)0xc7;
  *p++ = (char)0x04;
  *p++ = (char)0x25;
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

// Move imm64 to %rdi
static size_t emit_mov_imm64_rdi(long imm, char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;
  // mov imm, %rdi
  *p = (char)0x48;
  p++;
  *p = (char)0xbf;
  p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

// Move imm64 to %rsi
static size_t emit_mov_imm64_rsi(long imm, char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;
  // mov imm, %rsi
  *p = (char)0x48;
  p++;
  *p = (char)0xbe;
  p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

// Pass parameter to payload function
// Two parameters - POINT and Payload function
// If payload == 0, then we are dealing with single-process only
size_t SpSnippet::emit_pass_param(long point, long payload, char* buf,
                                  size_t offset) {
  assert(buf);
  char* p = buf + offset;
  size_t insnsize = 0;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point, p, 0);
  p += insnsize;

  if (payload) {
    // movq payload, %rsi
    insnsize = emit_mov_imm64_rsi((long)payload, p, 0);
    p += insnsize;
  }

  return (p - (buf + offset));
}

// Emulate to push an imm64 in stack
static size_t emit_push_imm64(long imm, char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;

  // push imm16
  // push imm16
  // push imm16
  // push imm16
  // ret
  for (int i = 3; i >= 0; i--) {
    short word = static_cast<unsigned short>((imm >> (16 * i)) & 0xffff);

    *p++ = 0x66;  // operand size override
    *p++ = 0x68;  // push immediate (16-bits b/c of prefix)
    *(short*)p = word;
    p += 2;
  }
  return (p - (buf + offset));
}

// Call a function w/ address `callee`
// Assumption: for 5-byte relative call instruction only
size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset, bool) {
  assert(buf);
  char* p = buf + offset;

  dt::Address retaddr = (dt::Address)p + 5;
  dt::Address rel_addr = (callee - retaddr);

  // Case 1: we are lucky to use relative call instruction.
  if (sp::IsDisp32(rel_addr)) {
    // call `callee`
    *p++ = 0xe8;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  }

  // Case 2: we have to use indirect call, because the function is
  // too far away.
  else {
    // Use %r11, because r11 is caller-saved

    // movq call_addr, %r11
    *p++ = 0x49;
    *p++ = 0xbb;
    long* call_addr = (long*)p;
    *call_addr = callee;
    p += sizeof(long);

    // call %r11
    *p++ = 0x41;
    *p++ = 0xff;
    *p++ = 0xd3;
  }

  return (p - (buf + offset));
}

// Jump to target address `trg`.
size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset, bool abs) {
  assert(buf);
  char* p = buf + offset;
  size_t insnsize = 0;

  dt::Address retaddr = (dt::Address)p + 5;
  dt::Address rel_addr = (trg - retaddr);

  if (sp::IsDisp32(rel_addr) && !abs) {
    // Case 1: we are lucky to use relative jump.

    // jmp trg
    *p++ = 0xe9;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    // Case 2: we have to emulate a long jump.

    // push jump target
    insnsize = emit_push_imm64(trg, p, 0);
    p += insnsize;

    // ret
    *p++ = 0xc3;
  }
  return (p - (buf + offset));
}

// Copied from dyninstAPI/src/emit-x86.c and modified
size_t SpSnippet::emitMovImmToReg64(Register dest, long imm, bool is_64,
                                    char* buf, size_t offset) {
  char* p = buf + offset;
  Register tmp_dest = dest;
  p += emitRex(is_64, NULL, NULL, &tmp_dest, p, 0);
  if (is_64) {
    *p++ = static_cast<char>(0xB8 + tmp_dest);
    *((long*)p) = imm;
    p += sizeof(long);
  }
  return (p - (buf + offset));
}

size_t SpSnippet::emitRex(bool is_64, Register* r, Register* x, Register* b,
                          char* buf, size_t offset) {
  char* p = buf + offset;
  char rex = 0x40;

  // need rex for 64-bit ops in most cases
  if (is_64) rex |= 0x08;

  // need rex for use of new registers
  // if a new register is used, we mask off the high bit before
  // returning since we account for it in the rex prefix

  // "R" register - extension to ModRM reg field
  if (r && *r & 0x08) {
    rex |= 0x04;
    *r &= 0x07;
  }

  // "X" register - extension to SIB index field
  if (x && *x & 0x08) {
    rex |= 0x02;
    *x &= 0x07;
  }

  // "B" register - extension to ModRM r/m field, SIB base field,
  // or opcode reg field
  if (b && *b & 0x08) {
    rex |= 0x01;
    *b &= 0x07;
  }
  // emit the rex, if needed
  // (note that some other weird cases not covered here
  //  need a "blank" rex, like using %sil or %dil)
  if (rex & 0x0f) *p++ = static_cast<char>(rex);

  return (p - (buf + offset));
}

size_t SpSnippet::emitPopReg64(Register dest, char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;
  p += emitRex(false, NULL, NULL, &dest, buf, offset);
  *p++ = static_cast<char>(0x58 + dest);
  return (p - (buf + offset));
}

size_t SpSnippet::emitPushReg64(Register src, char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;
  p += emitRex(false, NULL, NULL, &src, buf, offset);
  *p++ = static_cast<char>(0x50 + src);
  return (p - (buf + offset));
}

// Miscellaneous stuffs

// Used in trap handler to decide the pc value right at the call
dt::Address SpSnippet::get_pre_signal_pc(void* context) {
  assert(context);
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_RIP];
}

// Used in trap handler to jump to snippet
dt::Address SpSnippet::set_pc(dt::Address pc, void* context) {
  assert(context);
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_RIP] = pc;
  return pc;
}

// Used in trap handler to align the stack (pop return address of the stack)
dt::Address SpSnippet::align_stack(void* context) {
  assert(context);
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_RSP] -= 8;
  return ctx->uc_mcontext.gregs[REG_RSP];
}

// Get the saved register, for resolving indirect call
dt::Address SpSnippet::GetSavedReg(dt::MachRegister reg) {
#define RAX (8)
#define R9 (16)
#define R8 (24)
#define RCX (32)
#define RDX (40)
#define RSI (48)
#define RDI (56)
#define RSP (64)

#define R10 (-8)
#define R11 (-16)
#define RBX (-24)
#define R12 (-32)
#define R13 (-40)
#define R14 (-48)
#define R15 (-56)
#define RBP (-64)

  /*
    for (int i = -64; i < 48; i++) {
    sp_debug("DUMP SAVED REGS: %lx", reg_val((i*8)));
    }
  */
  dt::Address out = 0;
  if (GetRegInternal(reg, &out)) {
    sp_debug("GOT REG - %s = %lx", reg.name().c_str(), out);
    return out;
  }

  // RSP should be handled with special care, because we don't really
  // save %rsp in the stack
  namespace d64 = Dyninst::x86_64;

  if (reg == d64::rsp || reg == d64::esp || reg == d64::sp || reg == d64::spl) {
    sp_debug("GOT SP - %s = %lx = %lx + %x", reg.name().c_str(),
             saved_context_loc_ + RSP, saved_context_loc_, RSP);
    if (func()) {
      sp_debug("Before %s", func()->name().c_str());
    }
    return (saved_context_loc_ + RSP);
  }

  sp_debug("NOT FOUND - %s", reg.name().c_str());
  return 0;
}

class RelocVisitor : public in::Visitor {
 public:
  RelocVisitor(SpParser::ptr p) : in::Visitor(), p_(p), use_pc_(false) {}
  virtual void visit(in::RegisterAST* r) {
    assert(r);
    //      if (r->getID().isPC()) {
    if (r->getID() == Dyninst::x86_64::rip) {
      use_pc_ = true;
    }
  }
  virtual void visit(in::BinaryFunction* b) {}
  virtual void visit(in::Immediate* i) {}
  virtual void visit(in::Dereference* d) {}
  bool use_pc() const { return use_pc_; }

 private:
  SpParser::ptr p_;
  bool use_pc_;
};

/* Brief of X86 instruction format
   1. Instruction format
   | REX prefix (1B) | ESCAPE (1B) | Opcode (1~3B) | ModRM (1B) | SIB (1B) |
   Displacement | IMM |

   2. REX prefix ( 1 bytes)
   | 0100 | WRXB |

   W: if 1, then in 64-bit operand size
   R: concatenate w/ REG field in ModRM
   X: if 1, then use SIB
   B: concatenate w/ RM field in ModRM

   3. ModRM (1 byte)
   | Mod (2-bit) | REG (3-bit) | RM (3-bit)|

   RM: e.g., 101 means disp32(%rip)
   REG table:
   | REG | name |        | REG  | name |
   | 000 | rax  |        | 1000 | r8   |
   | 001 | rcx  |        | 1001 | r9   |
   | 010 | rdx  |        | 1010 | r10  |
   | 011 | rbx  |        | 1011 | r11  |
   | 100 | rsp  |        | 1100 | r12  |
   | 101 | rbp  |        | 1101 | r13  |
   | 110 | rsi  |        | 1110 | r14  |
   | 111 | rdi  |        | 1111 | r15  |
*/

// Get the displacement in an instruction- copied and modified from Dyninst
// directory common/src/arch-x86.C
/* static int*
 get_disp(in::Instruction::Ptr insn, char* insn_buf) {
   assert(insn);
   assert(insn_buf);
   int* disp = NULL;
   // Some SIMD instructions have a mandatory 0xf2 prefix;
   // 0xf3 as well...
   // Some 3-byte opcodes start with 0x66... skip

   int disp_offset = 0;
   if(insn->rawByte(disp_offset)==0x66) {
 disp_offset++;
   }
   if(insn->rawByte(disp_offset)==0xf2) {
 disp_offset++;
  }
   else if(insn->rawByte(disp_offset)==0xf3) {
 disp_offset++;
   }

   //Any REX instruction
   if ((insn_buf[disp_offset] & 0xf0) == 0x40) {
       disp_offset++;
   }


   //And "0x0f is a 2 byte opcode" skip
   if(insn_buf[disp_offset]==0x0f) {
 disp_offset++;
       // And the "0x38 or 0x3A is a 3-byte opcode" skip
       if(insn_buf[disp_offset] == 0x38 || insn_buf[disp_offset] == 0x3A)
   disp_offset++;
   }
   // Skip the instr opcode and the MOD/RM byte

   // OPCODE
   ++disp_offset;

   // ModRM
   ++disp_offset;

   disp = (int*)&insn_buf[disp_offset];
   return disp;
 }
*/
// This visitor visits a PC-sensitive call instruction
class EmuVisitor : public in::Visitor {
 public:
  EmuVisitor(dt::Address a) : Visitor(), imm_(0), a_(a) {}
  virtual void visit(in::RegisterAST* r) {
    assert(r);
    // value in RIP is a_
    if (r->getID() == Dyninst::x86_64::rip) {
      //      if (r->getID().isPC()) {
      imm_ = a_;
      stack_.push(imm_);
      sp_debug("EMU VISITOR - pc %lx", a_);
    }
  }
  virtual void visit(in::BinaryFunction* b) {
    assert(b);
    dt::Address i1 = stack_.top();
    stack_.pop();
    dt::Address i2 = stack_.top();
    stack_.pop();

    if (b->isAdd()) {
      imm_ = i1 + i2;
      sp_debug("EMU VISITOR - %lx + %lx = %lx", i1, i2, imm_);
    } else if (b->isMultiply()) {
      imm_ = i1 * i2;
      sp_debug("EMU VISITOR - %lx * %lx = %lx", i1, i2, imm_);
    } else {
      assert(0);
    }
    stack_.push(imm_);
  }
  virtual void visit(in::Immediate* i) {
    assert(i);
    in::Result res = i->eval();
    switch (res.size()) {
      case 1: {
        imm_ = res.val.u8val;
        break;
      }
      case 2: {
        imm_ = res.val.u16val;
        break;
      }
      case 4: {
        imm_ = res.val.u32val;
        break;
      }
      default: {
        imm_ = res.val.u64val;
        break;
      }
    }
    sp_debug("EMU VISITOR - IMM %lx", imm_);
    stack_.push(imm_);
  }
  virtual void visit(in::Dereference* d) {
    // Don't dereference for now
    // Should do it in runtime, not in instrumentation time
  }

  dt::Address imm() const { return imm_; }

 private:
  std::stack<dt::Address> stack_;
  dt::Address imm_;
  dt::Address a_;
};

// We emulate the instruction by this sequence:
//
// Case 1: if R8 is not used in this instruction
// push %r8
// mov IMM, %r8
// modified original instruction, and use %r8
// pop %r8
//
// Case 2: if R8 is used in this instruciton
// push %r9
// mov IMM, %r9, and use %r9
// modified original instruction
// pop %r9
//

/* static size_t
  emulate_pcsen(in::Instruction::Ptr insn,
                in::Expression::Ptr e,
                dt::Address a,
                char* buf) {
    assert(insn);
    assert(buf);
    char* p = buf;
    char* insn_buf = (char*)insn->ptr();
    assert(insn_buf);

    // Step 1: see if %r8 is used, so get register first

    // Get REX prefix, if it has one
    char rex = 0;
    int modrm_offset = 1;
    if ((insn_buf[0] & 0xf0) == 0x40) {
      rex = insn_buf[0];
      ++modrm_offset;
    }

    char escape = 0;
    if (insn_buf[modrm_offset-1] == 0x0f) {
      escape = 0x0f;
      ++modrm_offset;
    }

    // Get ModRM
    char modrm = insn_buf[modrm_offset];

    // Get the register used
    char reg = 0;
    if (rex) {
      // 64-bit reg
      if (rex & 0x04) reg |= 0x08;
    }
    reg |= ((modrm & 0x38) >> 3);

    // Step 2: push %r8 | push %r9

    // Push
    if (reg != 0x08) {
      // Case 1: Can use R8
      *p++ = 0x41; // push %r8
      *p++ = 0x50;
    } else {
      // Case 2: Cannot use R8, then use R9 instead.
      *p++ = 0x41; // push %r9
      *p++ = 0x51;
    }

    // Step 3: mov imm, %r8 | mov imm, %r9

    // Mov IMM64, %REG
    *p++ = 0x49;
    if (reg != 0x08) {
      *p++ = 0xb8; // mov imm64, %r8
    } else {
      *p++ = 0xb9; // mov imm64, %r9
    }
    long* l = (long*)p;


    // Deal with lea instruction
    if ((char)insn_buf[1] == (char)0x8d) {
      int* dis = get_disp(insn, insn_buf);

      sp_debug("LEA - orig-disp(%d), orig-insn-addr(%lx),"
               " orig-insn-size(%ld), abs-trg(%lx)",
               *dis, a, insn->size(), *dis+a+insn->size());

      *l =*dis + a + insn->size();
    }

    // Deal with non-lea instruction
    else {
      EmuVisitor visitor(a+insn->size());
      e->apply(&visitor);
      *l = visitor.imm();

      sp_debug("OTHER PC-INSN - orig-insn-size(%ld), abs-trg(%lx)",
               insn->size(), *l);

    }
    p += sizeof(*l);

    // Set rex
    if (rex) {
      rex |= 0x01;  // We use %r8 or %r9, so the last bit should be 1
      *p++ = rex;
    } else {
      *p++ = 0x41;  // If original no rex, then default it to be 32-bit thing
    }

    // Copy 0x0f
    if (escape) {
      *p++ = 0x0f;
    }

    // Copy opcode
    *p++ = insn_buf[modrm_offset-1];

    // Copy ModRM
    char new_modrm = modrm;
    if (reg != 0x08) {
      new_modrm &= 0xf8; // (R8), the last 3-bit should be 000
    } else {
      new_modrm &= 0xf9; // (R9), the last 3-bit should be 001
    }
    *p++ = new_modrm;

    // Copy imm after displacement
    for (unsigned i = modrm_offset+1+4; i < insn->size(); i++) {
      *p++ = insn_buf[i];
    }

    // Step 4: pop %r8 | pop %r9

    // Pop
    if (reg != 0x08) {
      // Case 1: Can use R8
      *p++ = 0x41; // pop %r8
      *p++ = 0x58;
    } else {
      // Case 2: Use R9 instead
      *p++ = 0x41; // pop %r9
      *p++ = 0x59;
    }

    return (size_t)(p - buf);
  }
*/
bool SpSnippet::getTargetAddr(dt::Address a, in::Instruction insn,
                              dt::Address& targetAddr) {
  // first get the target address associated with this instruction
  // Code copied from dynsintAPI/src/Relocation/Transformers/Movement-adhoc.C

  dt::Architecture fixme = insn.getArch();
  if (fixme == dt::Arch_ppc32) fixme = dt::Arch_ppc64;
  in::Expression::Ptr thePC(
      new in::RegisterAST(dt::MachRegister::getPC(insn.getArch())));
  in::Expression::Ptr thePCFixme(
      new in::RegisterAST(dt::MachRegister::getPC(fixme)));

  set<in::Expression::Ptr> mems;
  insn.getMemoryReadOperands(mems);
  insn.getMemoryWriteOperands(mems);
  for (set<in::Expression::Ptr>::const_iterator iter = mems.begin();
       iter != mems.end(); ++iter) {
    in::Expression::Ptr exp = *iter;
    if (exp->bind(thePC.get(), in::Result(in::u64, a + insn.size())) ||
        exp->bind(thePCFixme.get(), in::Result(in::u64, a + insn.size()))) {
      // Bind succeeded, eval to get targetAddr address
      in::Result res = exp->eval();
      if (!res.defined) {
        sp_debug("ERROR: failed bind/eval at %lx", a);
        continue;
      }
      assert(res.defined);
      targetAddr = res.convert<dt::Address>();
      return true;
    }
  }
  // Didn't use the PC to read memory; thus we have to grind through
  // all the operands. We didn't do this directly because the
  // memory-topping deref stops eval...
  vector<in::Operand> operands;
  insn.getOperands(operands);
  for (vector<in::Operand>::iterator iter = operands.begin();
       iter != operands.end(); ++iter) {
    // If we can bind the PC, then we're in the operand
    // we want.
    in::Expression::Ptr exp = iter->getValue();
    if (exp->bind(thePC.get(), in::Result(in::u64, a + insn.size())) ||
        exp->bind(thePCFixme.get(), in::Result(in::u64, a + insn.size()))) {
      // Bind succeeded, eval to get target address
      in::Result res = exp->eval();
      assert(res.defined);
      targetAddr = res.convert<dt::Address>();
      return true;
    }
  }

  return false;
}

size_t SpSnippet::reloc_insn_internal(dt::Address a, in::Instruction insn,
                                      std::set<in::Expression::Ptr>& exp,
                                      bool use_pc, char* p) {
  assert(insn.ptr());
  assert(p);
  if (use_pc) {
    // Deal with PC-sensitive instruction
    // first get the target address associated with this instruction
    dt::Address targetAddr = 0;
    sp_debug("READ USE PC: %s",
             g_parser->DumpInsns((void*)insn.ptr(), insn.size()).c_str());
    if (!getTargetAddr(a, insn, targetAddr))
      sp_debug("Cannot obtan target address");
    sp_debug("Target address is %lx", targetAddr);
    // Copied and  modified from dynisntAPT/src/codegen-x86.C
    instruction ins(insn.ptr(), true);
    const unsigned char* origInsn = ins.ptr();
    unsigned insnType = ins.type();
    unsigned insnSz = ins.size();
    dt::Address from = (dt::Address)p;

    bool is_data_abs64 = false;
    unsigned nPrefixes = count_prefixes(insnType);
    signed long newDisp = targetAddr - from;

    unsigned nOpcodeBytes = 1;
    if (*(origInsn + nPrefixes) == 0x0F) {
      nOpcodeBytes = 2;
      // 3-byte opcode support
      if ((*(origInsn + nPrefixes) == 0x0F) &&
          (*(origInsn + nPrefixes + 1) == 0x38 ||
           *(origInsn + nPrefixes + 1) == 0x3A)) {
        nOpcodeBytes = 3;
      }
    }

    Register pointer_reg = (Register)-1;

    unsigned char* newInsn = (unsigned char*)p;

#if defined(arch_x86_64)
    if (!is_disp32(newDisp + insnSz) && !is_addr32(targetAddr)) {
      // Case C: replace with 64-bit.
      is_data_abs64 = true;
      unsigned char mod_rm = *(origInsn + nPrefixes + nOpcodeBytes);
      pointer_reg = (mod_rm & 0x38) != 0 ? 0 : 3;
      newInsn += emitPushReg64(pointer_reg, (char*)newInsn, 0);
      newInsn +=
          emitMovImmToReg64(pointer_reg, targetAddr, true, (char*)newInsn, 0);
    }
#endif

    const unsigned char* origInsnStart = origInsn;

    // In other cases, we can rewrite the insn directly; in the 64-bit case, we
    // still need to copy the insn, instead of copy_prefixes
    // copy_prefixes(origInsn, newInsn, insnType);
    for (unsigned u = 0; u < nPrefixes; u++) {
      *newInsn++ = *origInsn++;
    }
    from += nPrefixes;

    if (*origInsn == 0x0F) {
      *newInsn++ = *origInsn++;
      // 3-byte opcode support
      if (*origInsn == 0x38 || *origInsn == 0x3A) {
        *newInsn++ = *origInsn++;
      }
    }

    // And the normal opcode
    *newInsn++ = *origInsn++;

    if (is_data_abs64) {
      // change ModRM byte to use [pointer_reg]: requires
      // us to change last three bits (the r/m field)
      // to the value of pointer_reg
      unsigned char mod_rm = *origInsn++;
      assert(pointer_reg != (Register)-1);
      mod_rm = (mod_rm & 0xf8) + pointer_reg;
      *newInsn++ = mod_rm;

    } else if (is_disp32(newDisp + insnSz)) {
      // Whee easy case
      *newInsn++ = *origInsn++;
      // Size doesn't change....
      *((int*)newInsn) = (int)(newDisp - insnSz);
      newInsn += 4;
    } else if (is_addr32(targetAddr)) {
      sp_debug("Target address is 32 bit");
      assert(!is_disp32(newDisp + insnSz));
      unsigned char mod_rm = *origInsn++;

      // change ModRM byte to use SIB addressing (r/m == 4)
      mod_rm = (mod_rm & 0xf8) + 4;
      *newInsn++ = mod_rm;

      // SIB == 0x25 specifies [disp32] addressing when mod == 0
      *newInsn++ = 0x25;

      // now throw in the displacement (the absolute 32-bit address)
      *((int*)newInsn) = (int)(targetAddr);
      newInsn += 4;
    } else {
      // Should never be reached...
      sp_debug("Reaching what should not be reached");
      assert(0);
    }

    // there may be an immediate after the displacement for RIP-relative
    // so we copy over the rest of the instruction here
    origInsn += 4;
    while (origInsn - origInsnStart < (int)insnSz) {
      *newInsn++ = *origInsn++;
    }

#if defined(arch_x86_64)
    if (is_data_abs64) {
      // Cleanup on aisle pointer_reg...
      assert(pointer_reg != (Register)-1);
      newInsn += emitPopReg64(pointer_reg, (char*)newInsn, 0);
    }
#endif
    return (newInsn - (unsigned char*)p);

  } else {
    // For non-pc-sensitive and non-last instruction, just copy it
    memcpy(p, insn.ptr(), insn.size());
    return insn.size();
  }
}

// Relocate an ordinary instruction
//
// The rule:
// 1. We skip the last insn in the block, which would be a call
//    insn, and we'll do it later.
// 2. For non-lea insn, we rely on memory read/write operands to
//    determine wheheter it is pc-relative insn
// 3. For lea insn, we use readSet/writeSet to determine if it is
//    a pc-relative insn

size_t SpSnippet::reloc_insn(dt::Address src_insn, in::Instruction insn,
                             dt::Address last, char* buf) {
  assert(insn.ptr());
  assert(buf);
  // We don't handle last instruction for now
  if (src_insn == last) {
    return 0;
  }

  // See if this instruction is a pc-sensitive instruction
  set<in::Expression::Ptr> opSet;
  bool use_pc = false;

  // Non-lea
  char* insn_buf = (char*)insn.ptr();
  assert(insn_buf);
  if ((char)insn_buf[1] != (char)0x8d) {
    if (insn.readsMemory())
      insn.getMemoryReadOperands(opSet);
    else if (insn.writesMemory())
      insn.getMemoryWriteOperands(opSet);

    for (set<in::Expression::Ptr>::iterator i = opSet.begin(); i != opSet.end();
         i++) {
      RelocVisitor visitor(g_parser);
      (*i)->apply(&visitor);
      use_pc = visitor.use_pc();
    }
  }
  // lea instruction
  else {
    set<in::RegisterAST::Ptr> pcExp;

    bool read_use_pc = false;
    insn.getReadSet(pcExp);
    for (set<in::RegisterAST::Ptr>::iterator i = pcExp.begin();
         i != pcExp.end(); i++) {
      RelocVisitor visitor(g_parser);
      (*i)->apply(&visitor);
      read_use_pc = visitor.use_pc();
      if (read_use_pc) {
        opSet.insert(*i);
        break;
      }
    }

    bool write_use_pc = false;
    insn.getWriteSet(pcExp);
    for (set<in::RegisterAST::Ptr>::iterator i = pcExp.begin();
         i != pcExp.end(); i++) {
      RelocVisitor visitor(g_parser);
      (*i)->apply(&visitor);
      write_use_pc = visitor.use_pc();
      if (write_use_pc) {
        opSet.insert(*i);
        break;
      }
    }

    use_pc = (read_use_pc || write_use_pc);
  }

  if (use_pc) {
    assert(opSet.size() == 1);
    sp_debug("USE PC: %s",
             g_parser->DumpInsns((void*)insn.ptr(), insn.size()).c_str());
    sp_debug("USE_PC - at %lx", src_insn);
  } else {
    sp_debug("NOT_USE PC - at %lx", src_insn);
  }

  // Here we go!
  return reloc_insn_internal(src_insn, insn, opSet, use_pc, buf);
}

// The upper bound for a jump instruction.
size_t SpSnippet::jump_abs_size() {
  // push x 4
  // ret
  return 17;
}

// Relocate the call instruction
// This is used in deadling with indirect call
size_t SpSnippet::emit_call_orig(char* buf, size_t offset) {
  assert(buf);
  char* p = buf + offset;
  SpBlock* b = point_->GetBlock();
  assert(b);
  long src = b->last();

  in::Instruction insn = b->orig_call_insn();
  assert(insn.ptr());
  set<in::Expression::Ptr> opSet;
  bool use_pc = false;

  // Check whether the call instruction uses RIP
  RelocVisitor visitor(g_parser);
  in::Expression::Ptr trg = insn.getControlFlowTarget();
  if (trg) {
    trg->apply(&visitor);
    use_pc = visitor.use_pc();
    opSet.insert(trg);
  }

  sp_debug("EMIT_CALL_ORIG - for call insn at %lx", src);
  sp_debug("BEFORE RELOC - %s",
           g_parser->DumpInsns((void*)insn.ptr(), insn.size()).c_str());
  if (use_pc) sp_debug("PC-REL CALL");
  size_t insn_size = reloc_insn_internal(src, insn, opSet, use_pc, p);
  sp_debug("AFTER RELOC %s", g_parser->DumpInsns((void*)p, insn_size).c_str());
  return insn_size;
}

// Get argument of a function call
void* SpSnippet::PopArgument(ArgumentHandle* h, size_t size) {
  assert(h);
  using namespace Dyninst::x86_64;
  if (h->num < 6) {
    dt::Address a = 0;
    switch (h->num) {
      case 0:
        a = GetSavedReg(rdi);
        break;
      case 1:
        a = GetSavedReg(rsi);
        break;
      case 2:
        a = GetSavedReg(rdx);
        break;
      case 3:
        a = GetSavedReg(rcx);
        break;
      case 4:
        a = GetSavedReg(r8);
        break;
      case 5:
        a = GetSavedReg(r9);
        break;
      default:
        assert(0);
    }
    char* b = h->insert_buf(size);
    memcpy(b, &a, size);
    ++h->num;
    return b;
  }

  void* a = (void*)(saved_context_loc_ + RSP + h->offset);
  h->offset += size;
  ++h->num;
  return a;
}

// Get return value of a function call
long SpSnippet::GetRetVal() { return GetSavedReg(Dyninst::x86_64::rax); }

void SpSnippet::InitSavedRegMap() {
  namespace d64 = Dyninst::x86_64;

  saved_reg_map_[d64::rax] = RAX;
  saved_reg_map_[d64::eax] = RAX;
  saved_reg_map_[d64::ax] = RAX;
  saved_reg_map_[d64::ah] = RAX;
  saved_reg_map_[d64::al] = RAX;

  saved_reg_map_[d64::rbx] = RBX;
  saved_reg_map_[d64::ebx] = RBX;
  saved_reg_map_[d64::bx] = RBX;
  saved_reg_map_[d64::bh] = RBX;
  saved_reg_map_[d64::bl] = RBX;

  saved_reg_map_[d64::rcx] = RCX;
  saved_reg_map_[d64::ecx] = RCX;
  saved_reg_map_[d64::cx] = RCX;
  saved_reg_map_[d64::ch] = RCX;
  saved_reg_map_[d64::cl] = RCX;

  saved_reg_map_[d64::rdx] = RDX;
  saved_reg_map_[d64::edx] = RDX;
  saved_reg_map_[d64::dx] = RDX;
  saved_reg_map_[d64::dh] = RDX;
  saved_reg_map_[d64::dl] = RDX;

  saved_reg_map_[d64::rsi] = RSI;
  saved_reg_map_[d64::esi] = RSI;
  saved_reg_map_[d64::si] = RSI;
  saved_reg_map_[d64::sil] = RSI;

  saved_reg_map_[d64::rdi] = RDI;
  saved_reg_map_[d64::edi] = RDI;
  saved_reg_map_[d64::di] = RDI;
  saved_reg_map_[d64::dil] = RDI;

  saved_reg_map_[d64::rbp] = RBP;
  saved_reg_map_[d64::ebp] = RBP;
  saved_reg_map_[d64::bp] = RBP;
  saved_reg_map_[d64::bpl] = RBP;

  saved_reg_map_[d64::r8] = R8;
  saved_reg_map_[d64::r8d] = R8;
  saved_reg_map_[d64::r8w] = R8;
  saved_reg_map_[d64::r8b] = R8;

  saved_reg_map_[d64::r9] = R9;
  saved_reg_map_[d64::r9d] = R9;
  saved_reg_map_[d64::r9w] = R9;
  saved_reg_map_[d64::r9b] = R9;

  saved_reg_map_[d64::r10] = R10;
  saved_reg_map_[d64::r10d] = R10;
  saved_reg_map_[d64::r10w] = R10;
  saved_reg_map_[d64::r10b] = R10;

  saved_reg_map_[d64::r11] = R11;
  saved_reg_map_[d64::r11d] = R11;
  saved_reg_map_[d64::r11w] = R11;
  saved_reg_map_[d64::r11b] = R11;

  saved_reg_map_[d64::r12] = R12;
  saved_reg_map_[d64::r12d] = R12;
  saved_reg_map_[d64::r12w] = R12;
  saved_reg_map_[d64::r12b] = R12;

  saved_reg_map_[d64::r13] = R13;
  saved_reg_map_[d64::r13d] = R13;
  saved_reg_map_[d64::r13w] = R13;
  saved_reg_map_[d64::r13b] = R13;

  saved_reg_map_[d64::r14] = R14;
  saved_reg_map_[d64::r14d] = R14;
  saved_reg_map_[d64::r14w] = R14;
  saved_reg_map_[d64::r14b] = R14;

  saved_reg_map_[d64::r15] = R15;
  saved_reg_map_[d64::r15d] = R15;
  saved_reg_map_[d64::r15w] = R15;
  saved_reg_map_[d64::r15b] = R15;
}

dt::Address SpSnippet::GetFs(void* context) {
  assert(context);
  ucontext_t* ctx = ((ucontext_t*)context);
  assert(ctx);
  // REG_CSGSFS
  int index = REG_CSGSFS;
  sp_debug("REG_CSGSFS=%llx", ctx->uc_mcontext.gregs[index]);
  return ((ctx->uc_mcontext.gregs[index] >> 32) & 0xffff);
}

// Examine whether an instruction is using Pc-sensitive insn

class UsePcVisitor : public in::Visitor {
 public:
  UsePcVisitor() : in::Visitor(), use_pc_(false) {}
  virtual void visit(in::RegisterAST* r) {
    assert(r);
    // if (r->getID().isPC()) {
    if (r->getID() == Dyninst::x86_64::rip) {
      use_pc_ = true;
    }
  }
  virtual void visit(in::BinaryFunction* b) {}
  virtual void visit(in::Immediate* i) {}
  virtual void visit(in::Dereference* d) {}
  bool use_pc() const { return use_pc_; }

 private:
  bool use_pc_;
};

bool SpSnippet::UsePC(in::Instruction insn) {
  assert(insn.ptr());

  set<in::RegisterAST::Ptr> pcExp;
  bool read_use_pc = false;
  insn.getReadSet(pcExp);
  for (set<in::RegisterAST::Ptr>::iterator i = pcExp.begin(); i != pcExp.end();
       i++) {
    UsePcVisitor visitor;
    (*i)->apply(&visitor);
    read_use_pc = visitor.use_pc();
    if (read_use_pc) {
      sp_debug("READ USE PC: %s",
               g_parser->DumpInsns((void*)insn.ptr(), insn.size()).c_str());
      return true;
    }
  }

  bool write_use_pc = false;
  insn.getWriteSet(pcExp);
  for (set<in::RegisterAST::Ptr>::iterator i = pcExp.begin(); i != pcExp.end();
       i++) {
    UsePcVisitor visitor;
    (*i)->apply(&visitor);
    write_use_pc = visitor.use_pc();
    if (write_use_pc) {
      sp_debug("WRITE USE PC: %s",
               g_parser->DumpInsns((void*)insn.ptr(), insn.size()).c_str());
      return true;
    }
  }

  return false;
}
}  // namespace sp
