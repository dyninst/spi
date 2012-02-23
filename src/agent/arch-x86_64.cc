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

#include <stack>

#include "common/common.h"

#include "agent/event.h"
#include "agent/point.h"
#include "agent/parser.h"
#include "agent/snippet.h"
#include "agent/context.h"
#include "agent/instrumenter.h"
#include "agent/inst_worker_delegate.h"
#include "agent/trap_worker_impl.h"
#include "agent/callinsn_worker_impl.h"
#include "agent/callblk_worker_impl.h"
#include "agent/spring_worker_impl.h"

#include "common/utils.h"

#include "instructionAPI/h/Visitor.h"
#include "instructionAPI/h/Immediate.h"
#include "instructionAPI/h/BinaryFunction.h"

namespace sp {

	extern SpContext* g_context;
	extern SpParser::ptr g_parser;


  // Code Generation stuffs

  // Save context before calling payload
  size_t
  SpSnippet::emit_save(char* buf,
											 size_t offset) {
		assert(buf);
    char* p = buf + offset;

		bool indirect = false;
		if (!func_) {
			indirect = true;
		}

    // Saved for direct/indirect call
    *p++ = 0x57; // push rdi
    *p++ = 0x56; // push rsi
    *p++ = 0x52; // push rdx
    *p++ = 0x51; // push rcx
    *p++ = 0x41; // push r8
    *p++ = 0x50;
    *p++ = 0x41; // push r9
    *p++ = 0x51;
    *p++ = 0x50; // push rax

    // Save stack pointer at this point, for future lookup for other
    // saved registers at runtime.
    p += emit_save_sp(p, 0);

    // Saved for indirect call only
    if (indirect) {
      *p++ = 0x41; // push r10 -- unused in C
      *p++ = 0x52;
      *p++ = 0x41; // push r11 -- for linker
      *p++ = 0x53;
      *p++ = 0x53; // push rbx
      *p++ = 0x41; // push r12
      *p++ = 0x54;
      *p++ = 0x41; // push r13
      *p++ = 0x55;
      *p++ = 0x41; // push r14
      *p++ = 0x56;
      *p++ = 0x41; // push r15
      *p++ = 0x57;
      *p++ = 0x55; // push rbp
    }

    return (p - (buf + offset));
  }

  // Restore context after calling payload
  size_t
  SpSnippet::emit_restore(char* buf,
													size_t offset) {
		assert(buf);
    char* p = buf + offset;

		bool indirect = false;
		if (!func_) {
			indirect = true;
		}

    // Restored for indirect call
    if (indirect) {
      *p++ = 0x5d; // pop rbp
      *p++ = 0x41; // pop r15
      *p++ = 0x5f;
      *p++ = 0x41; // pop r14
      *p++ = 0x5e;
      *p++ = 0x41; // pop r13
      *p++ = 0x5d;
      *p++ = 0x41; // pop r12
      *p++ = 0x5c;
      *p++ = 0x5b; // pop rbx
      *p++ = 0x41; // pop r11
      *p++ = 0x5b;
      *p++ = 0x41; // pop r10
      *p++ = 0x5a;
    }

    // Restored for direct/indirect call
    *p++ = 0x58; // pop rax
    *p++ = 0x41; // pop r9
    *p++ = 0x59;
    *p++ = 0x41; // pop r8
    *p++ = 0x58;
    *p++ = 0x59; // pop rcx
    *p++ = 0x5a; // pop rdx
    *p++ = 0x5e; // pop rsi
    *p++ = 0x5f; // pop rdi

    return (p - (buf + offset));
  }

  // Save stack pionter, for two purposes
  // 1. Resolve indirect call during runtime
  // 2. Get argument of callees in payload function
  size_t
  SpSnippet::emit_save_sp(char* buf,
													size_t offset) {
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
  size_t
  SpSnippet::emit_fault(char* buf,
												size_t offset) {
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
  static size_t
  emit_mov_imm64_rdi(long imm,
										 char* buf,
										 size_t offset) {
		assert(buf);
    char* p = buf + offset;
    // mov imm, %rdi
    *p = (char)0x48; p++;
    *p = (char)0xbf; p++;
    *((long*)p) = (long)imm;
    return (2 + sizeof(long));
  }

  // Move imm64 to %rsi
  static size_t
  emit_mov_imm64_rsi(long imm,
										 char* buf,
										 size_t offset) {
		assert(buf);
    char* p = buf + offset;
    // mov imm, %rsi
    *p = (char)0x48; p++;
    *p = (char)0xbe; p++;
    *((long*)p) = (long)imm;
    return (2 + sizeof(long));
  }

  // Pass parameter to payload function
  // Two parameters - POINT and Payload function
  // If payload == 0, then we are dealing with single-process only
  size_t
  SpSnippet::emit_pass_param(long point,
														 long payload,
														 char* buf,
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
  static size_t
  emit_push_imm64(long imm,
									char* buf,
									size_t offset) {
		assert(buf);
    char* p = buf + offset;

    // push imm16
    // push imm16
    // push imm16
    // push imm16
    // ret
    for (int i = 3; i >= 0; i--) {
      short word = static_cast<unsigned short>((imm >> (16 * i)) & 0xffff);
      *p++ = 0x66; // operand size override
      *p++ = 0x68; // push immediate (16-bits b/c of prefix)
      *(short *)p = word;
      p += 2;
    }
    return (p - (buf + offset));
  }

  // Call a function w/ address `callee`
  // Assumption: for 5-byte relative call instruction only
  size_t
  SpSnippet::emit_call_abs(long callee,
													 char* buf,
													 size_t offset,
													 bool) {
		assert(buf);
    char* p = buf + offset;

    // Case 1: we are lucky to use relative call instruction.
    dt::Address retaddr = (dt::Address)p+5;
    dt::Address rel_addr = (callee - retaddr);
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
      // Use %r15, because r15 is callee-saved

      // push r15
      *p++ = 0x41;
      *p++ = 0x57;

      // movq call_addr, %r15
      *p++ = 0x49;
      *p++ = 0xbf;
      long* call_addr = (long*)p;
      *call_addr = callee;
      p += sizeof(long);

      // call %r15
      *p++ = 0x41;
      *p++ = 0xff;
      *p++ = 0xd7;

      // pop r15
      *p++ = 0x41;
      *p++ = 0x5f;
    }

    return (p - (buf + offset));
  }

  // Jump to target address `trg`.
  size_t
  SpSnippet::emit_jump_abs(long trg,
													 char* buf,
													 size_t offset,
													 bool abs) {
		assert(buf);
    char* p = buf + offset;
    size_t insnsize = 0;

    dt::Address retaddr = (dt::Address)p+5;
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


  // Miscellaneous stuffs

  // Used in trap handler to decide the pc value right at the call
  dt::Address
  SpSnippet::get_pre_signal_pc(void* context) {
		assert(context);
    ucontext_t* ctx = (ucontext_t*)context;
    return ctx->uc_mcontext.gregs[REG_RIP];
  }

  // Used in trap handler to jump to snippet
  dt::Address
  SpSnippet::set_pc(dt::Address pc, void* context) {
		assert(context);
    ucontext_t* ctx = (ucontext_t*)context;
    ctx->uc_mcontext.gregs[REG_RIP] = pc;
    return pc;
  }

  // Get the saved register, for resolving indirect call
  dt::Address
  SpSnippet::get_saved_reg(Dyninst::MachRegister reg) {

#define RAX (0)
#define R9 (8)
#define R8 (16)
#define RCX (24)
#define RDX (32)
#define RSI (40)
#define RDI (48)
#define RSP (56)

#define R10 (-8)
#define R11 (-16)
#define RBX (-24)
#define R12 (-32)
#define R13 (-40)
#define R14 (-48)
#define R15 (-56)
#define RBP (-64)

#define reg_val(i) (*(long*)(saved_context_loc_+(long)(i)))

    /*
      for (int i = -64; i < 48; i++) {
      sp_debug("DUMP SAVED REGS: %lx", reg_val((i*8)));
      }
    */
    using namespace Dyninst::x86_64;

    if (reg == rax) return reg_val(RAX);
    if (reg == rbx) return reg_val(RBX);
    if (reg == rcx) return reg_val(RCX);
    if (reg == rdx) return reg_val(RDX);
    if (reg == rsi) return reg_val(RSI);
    if (reg == rdi) return reg_val(RDI);
    if (reg == r8) return reg_val(R8);
    if (reg == r9) return reg_val(R9);
    if (reg == r10) return reg_val(R10);
    if (reg == r11) return reg_val(R11);
    if (reg == r12) return reg_val(R12);
    if (reg == r13) return reg_val(R13);
    if (reg == r14) return reg_val(R14);
    if (reg == r15) return reg_val(R15);
    if (reg == rsp) return (saved_context_loc_+RSP);
    //if (reg == rsp) return reg_val(RSP);
    if (reg == rbp) return reg_val(RBP);

    if (reg == eax) return reg_val(RAX);
    if (reg == ebx) return reg_val(RBX);
    if (reg == ecx) return reg_val(RCX);
    if (reg == edx) return reg_val(RDX);
    if (reg == esi) return reg_val(RSI);
    if (reg == edi) return reg_val(RDI);
    if (reg == esp) return (saved_context_loc_+RSP);
    //if (reg == esp) return reg_val(RSP);
    if (reg == ebp) return reg_val(RBP);

    return 0;
  }

  // Is this register RIP?
  bool
  IsPcRegister(Dyninst::MachRegister r) {
    if (r == Dyninst::x86_64::rip) return true;
    return false;
  }

  class RelocVisitor : public in::Visitor {
  public:
    RelocVisitor(SpParser::ptr p)
			: in::Visitor(), p_(p), use_pc_(false) {}
    virtual void visit(in::RegisterAST* r) {
      // sp_debug("USE REG");
			assert(r);
      if (IsPcRegister(r->getID())) {
        use_pc_ = true;
      }
    }
    virtual void visit(in::BinaryFunction* b) {
    }
    virtual void visit(in::Immediate* i) {
    }
    virtual void visit(in::Dereference* d) {
    }
    bool use_pc() const { return use_pc_; }
  private:
    SpParser::ptr p_;
    bool use_pc_;
  };

  /* Brief of X86 instruction format
     1. Instruction format
     | REX prefix (1B) | ESCAPE (1B) | Opcode (1~3B) | ModRM (1B) | SIB (1B) | Displacement | IMM |

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

  // Get the displacement in an instruction
  static int*
  get_disp(in::Instruction::Ptr insn, char* insn_buf) {
		assert(insn);
		assert(insn_buf);
    int* disp = NULL;

    int disp_offset = 0;
    // Any REX?
    if ((insn_buf[disp_offset] & 0xf0) == 0x40) {
      ++disp_offset;
    }

    // Any ESCAPE?
    if (insn_buf[disp_offset] == 0x0f) {
      ++disp_offset;
    }

    // OPCODE
    ++disp_offset;

    // ModRM
    ++disp_offset;

    disp = (int*)&insn_buf[disp_offset];
    return disp;
  }

  // This visitor visits a PC-sensitive call instruction
  class EmuVisitor : public in::Visitor {
  public:
    EmuVisitor(dt::Address a)
      : Visitor(), imm_(0), a_(a) { }
    virtual void visit(in::RegisterAST* r) {
			assert(r);
      // value in RIP is a_
			if (IsPcRegister(r->getID())) {
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
        imm_ =res.val.u8val;
        break;
      }
      case 2: {
        imm_ =res.val.u16val;
        break;
      }
      case 4: {
        imm_ =res.val.u32val;
        break;
      }
      default: {
        imm_ =res.val.u64val;
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

    dt::Address imm() const {
      return imm_;
    }

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

  static size_t
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

  static size_t
  reloc_insn_internal(dt::Address a,
											in::Instruction::Ptr insn,
                      std::set<in::Expression::Ptr>& exp,
                      bool use_pc,
											char* p) {
		assert(insn);
		assert(p);
    if (use_pc) {
      // Deal with PC-sensitive instruction
      char insn_buf[20];
      memcpy(insn_buf, insn->ptr(), insn->size());
      int* dis_buf = get_disp(insn, insn_buf);
      long old_rip = a;
      long new_rip = (long)p;
      long long_new_dis = (old_rip - new_rip) + *dis_buf;

      sp_debug("RELOC INSN - insn addr %lx, old_rip %lx, new_rip %lx,"
               " displacement %x", a, old_rip,new_rip, *dis_buf);
      sp_debug("RELOC INSN - new displacement %lx", long_new_dis);

      if (sp::IsDisp32(long_new_dis)) {
        // Easy case: just modify the displacement
        *dis_buf = (int)long_new_dis;
        memcpy(p, insn_buf, insn->size());
        return insn->size();
      } else {
        // General purpose: emulate the instruction
        size_t insn_size = emulate_pcsen(insn, *exp.begin(), a, p);
        return insn_size;
      }
    } else {
      // For non-pc-sensitive and non-last instruction, just copy it
      memcpy(p, insn->ptr(), insn->size());
      return insn->size();
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

  size_t
  SpSnippet::reloc_insn(dt::Address src_insn,
												in::Instruction::Ptr insn,
                        dt::Address last,
												char* buf) {
		assert(insn);
		assert(buf);
    // We don't handle last instruction for now
    if (src_insn == last) {  return 0;  }

    // See if this instruction is a pc-sensitive instruction
    set<in::Expression::Ptr> opSet;
    bool use_pc = false;

		// Non-lea
		char* insn_buf = (char*)insn->ptr();
		assert(insn_buf);
		if ((char)insn_buf[1] != (char)0x8d) {
			if (insn->readsMemory()) insn->getMemoryReadOperands(opSet);
			else if (insn->writesMemory()) insn->getMemoryWriteOperands(opSet);

			for (set<in::Expression::Ptr>::iterator i = opSet.begin();
					 i != opSet.end(); i++) {
				RelocVisitor visitor(g_parser);
				(*i)->apply(&visitor);
				use_pc = visitor.use_pc();
			}
		}
		// lea instruction
		else {
			set<in::RegisterAST::Ptr> pcExp;

			bool read_use_pc = false;
			insn->getReadSet(pcExp);
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
			insn->getWriteSet(pcExp);
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
			sp_debug("USE_PC - at %lx", src_insn);
		} else {
			sp_debug("NOT_USE PC - at %lx", src_insn);
		}

    // Here we go!
    return reloc_insn_internal(src_insn, insn, opSet, use_pc, buf);
  }

  // The upper bound for a jump instruction.
  size_t
  SpSnippet::jump_abs_size() {
    // push x 4
    // ret
    return 17;
  }

  // Relocate the call instruction
  // This is used in deadling with indirect call
  size_t
  SpSnippet::emit_call_orig(char* buf,
														size_t offset) {
		assert(buf);
    char* p = buf + offset;
		SpBlock* b = point_->GetBlock();
		assert(b);
		long src = b->last();

		in::Instruction::Ptr insn = b->orig_call_insn();
		assert(insn);
    set<in::Expression::Ptr> opSet;
    bool use_pc = false;

    // Check whether the call instruction uses RIP
    RelocVisitor visitor(g_parser);
		in::Expression::Ptr trg = insn->getControlFlowTarget();
    if (trg) {
      trg->apply(&visitor);
      use_pc = visitor.use_pc();
      opSet.insert(trg);
    }

		sp_debug("EMIT_CALL_ORIG - for call insn at %lx", src);
    sp_debug("BEFORE RELOC - %s",
						 g_parser->dump_insn((void*)insn->ptr(),
															 insn->size()).c_str());
		if (use_pc) sp_debug("PC-REL CALL");
		size_t insn_size = reloc_insn_internal(src, insn, opSet, use_pc, p);
    sp_debug("AFTER RELOC %s", g_parser->dump_insn((void*)p,
																									 insn_size).c_str());
		return insn_size;
  }

  // Get argument of a function call
  void*
  SpSnippet::pop_argument(ArgumentHandle* h,
													size_t size) {
		assert(h);
    using namespace Dyninst::x86_64;
    if (h->num < 6) {
      dt::Address a = 0;
      switch(h->num) {
      case 0:
        a = get_saved_reg(rdi);
        break;
      case 1:
        a = get_saved_reg(rsi);
        break;
      case 2:
        a = get_saved_reg(rdx);
        break;
      case 3:
        a = get_saved_reg(rcx);
        break;
      case 4:
        a = get_saved_reg(r8);
        break;
      case 5:
        a = get_saved_reg(r9);
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
  long
  SpSnippet::get_ret_val() {
    return get_saved_reg(Dyninst::x86_64::rax);
  }


// The estimate blob size for different instrumentation worker

	size_t
	InstWorkerDelegate::BaseEstimateRelocInsnSize(SpPoint* pt) {
		return 100;
	}

	size_t
	InstWorkerDelegate::BaseEstimateRelocBlockSize(SpPoint* pt) {
		assert(pt);
		SpBlock* blk = pt->GetBlock();
		assert(blk);
		return blk->size() + InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
	}

	size_t
	TrapWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
	}

	size_t
	RelocCallInsnWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocInsnSize(pt);
	}

	size_t
	RelocCallBlockWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocBlockSize(pt);
	}

	size_t
	SpringboardWorker::EstimateBlobSize(SpPoint* pt) {
		return InstWorkerDelegate::BaseEstimateRelocBlockSize(pt);
	}
}
