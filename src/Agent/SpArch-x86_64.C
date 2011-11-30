#include <signal.h>
#include <ucontext.h>
#include <stack>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "SpPoint.h"
#include "SpUtils.h"

#include "Visitor.h"
#include "BinaryFunction.h"
#include "Immediate.h"

using Dyninst::PatchAPI::PatchFunction;
extern sp::SpContext* g_context;

namespace sp {

//==============================================================================
// Code generation
//==============================================================================

// Save context before calling payload
size_t SpSnippet::emit_save(char* buf, size_t offset, bool indirect) {
  char* p = buf + offset;

  // Saved for direct/indirect call
  //*p++ = 0x54; // push rsp
  *p++ = 0x57; // push rdi
  *p++ = 0x56; // push rsi
  *p++ = 0x52; // push rdx
  *p++ = 0x51; // push rcx
  *p++ = 0x41; // r8
  *p++ = 0x50;
  *p++ = 0x41; // r9
  *p++ = 0x51;
  *p++ = 0x50; // %rax

  //sp::SpPoint* spt = static_cast<sp::SpPoint*>(point_);
  //long* l = spt->saved_context_ptr();
  //size_t insnsize = emit_save_sp((long)l, p, 0);
  size_t insnsize = emit_save_sp(p, 0);
  p += insnsize;

  // Saved for indirect call
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
size_t SpSnippet::emit_restore(char* buf, size_t offset, bool indirect) {
  char* p = buf + offset;

  // Restored for indirect call
  if (indirect) {
    *p++ = 0x5d; // rbp
    *p++ = 0x41; // r15
    *p++ = 0x5f;
    *p++ = 0x41; // r14
    *p++ = 0x5e;
    *p++ = 0x41; // r13
    *p++ = 0x5d;
    *p++ = 0x41; // r12
    *p++ = 0x5c;
    *p++ = 0x5b; // rbx
    *p++ = 0x41; // r11
    *p++ = 0x5b;
    *p++ = 0x41; // r10
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
  //*p++ = 0x5c; // pop rsp

  return (p - (buf + offset));
}

// Save stack pionter, for two purposes
// 1. Resolve indirect call during runtime
// 2. Get argument of callees in payload function
size_t SpSnippet::emit_save_sp(char* buf, size_t offset) {
  char* p = buf + offset;

  *p++ = 0x48;  // mov loc, %rax
  *p++ = 0xb8;
  long* l = (long*)p;
  *l = (long)&saved_context_loc_;
  p += sizeof(long);

  *p++ = 0x48;  // mov %rsp, (%rax)
  *p++ = 0x89;
  *p++ = 0x20;

  return (p - (buf + offset));
}

// For debugging, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
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
  char* p = buf + offset;
  *p = (char)0x48; p++;  // mov imm, %rdi
  *p = (char)0xbf; p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

// Move imm64 to %rsi
static size_t emit_mov_imm64_rsi(long imm, char* buf, size_t offset) {
  char* p = buf + offset;
  *p = (char)0x48; p++;  // mov imm, %rsi
  *p = (char)0xbe; p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

// Pass parameter to payload function, which has only one parameter POINT
size_t SpSnippet::emit_pass_param(long point, long payload, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // movq payload, %rsi
  insnsize = emit_mov_imm64_rsi((long)payload, p, 0);
  p += insnsize;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point, p, 0);
  p += insnsize;

  return (p - (buf + offset));
}

// Save an imm32 in stack
static size_t emit_push_imm64(long imm, char* buf, size_t offset) {
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
size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset, bool) {
  char* p = buf + offset;
  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  Dyninst::Address rel_addr = (callee - retaddr);

  if (sp::is_disp32(rel_addr)) {
    *p++ = 0xe8;         // call callee
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    *p++ = 0x48;         // movq call_addr, %rax
    *p++ = 0xb8;
    long* call_addr = (long*)p;
    *call_addr = callee;
    p += sizeof(long);

    *p++ = 0xff;        // call %rax
    *p++ = 0xd0;
  }

  return (p - (buf + offset));
}

size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset, bool abs) {
  char* p = buf + offset;
  size_t insnsize = 0;

  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  Dyninst::Address rel_addr = (trg - retaddr);

  if (sp::is_disp32(rel_addr) && !abs) {
    *p++ = 0xe9;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    // push jump target
    insnsize = emit_push_imm64(trg, p, 0);
    p += insnsize;
    // ret
    *p++ = 0xc3;
  }
  return (p - (buf + offset));
}

//==============================================================================
// Miscellaneous
//==============================================================================

// Used in trap handler to decide the pc value right at the call
Dyninst::Address SpSnippet::get_pre_signal_pc(void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_RIP];
}

// Used in trap handler to jump to snippet
Dyninst::Address SpSnippet::set_pc(Dyninst::Address pc, void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_RIP] = pc;
}

// Get the saved register, for resolving indirect call
Dyninst::Address SpSnippet::get_saved_reg(Dyninst::MachRegister reg) {
  //sp_debug("INDIRECT - get saved register %s", reg.name().c_str());

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

  //sp_print("Cannot find register %s", reg.name().c_str());
  return 0;
}

// Is this register RIP?
bool SpParser::is_pc(Dyninst::MachRegister r) {
  //sp_debug("IS PC ? - %s", r.name().c_str());
  if (r == Dyninst::x86_64::rip) return true;
  return false;
}

using namespace Dyninst::InstructionAPI;
class RelocVisitor : public Visitor {
  public:
    RelocVisitor(SpParser::ptr p) : Visitor(), p_(p), use_pc_(false) {}
    virtual void visit(RegisterAST* r) {
      if (p_->is_pc(r->getID())) {
        use_pc_ = true;
      }
    }
    virtual void visit(BinaryFunction* b) {
    }
    virtual void visit(Immediate* i) {
    }
    virtual void visit(Dereference* d) {
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
static int* get_disp(Instruction::Ptr insn, char* insn_buf) {
  int* disp = NULL;

  int disp_offset = 0;
  // Any REX?
  if ((insn_buf[disp_offset] & 0xf0) == 0x40) {
    //sp_debug("GOT REX prefix - %x", insn_buf[disp_offset]);
    ++disp_offset;
  }

  // Any ESCAPE?
  if (insn_buf[disp_offset] == 0x0f) {
    //sp_debug("GOT REX prefix - %x", insn_buf[disp_offset]);
    ++disp_offset;
  }

  // OPCODE
  ++disp_offset;

  // MODRM
  ++disp_offset;

  disp = (int*)&insn_buf[disp_offset];
  return disp;
}

// This visitor visits a PC-sensitive call instruction
class EmuVisitor : public Visitor {
public:
  EmuVisitor(Dyninst::Address a)
    : Visitor(), imm_(0), a_(a) { }
  virtual void visit(RegisterAST* r) {
    imm_ = a_;  // value in RIP is a_
    stack_.push(imm_);
  }
  virtual void visit(BinaryFunction* b) {
    Dyninst::Address i1 = stack_.top();
    stack_.pop();
    Dyninst::Address i2 = stack_.top();
    stack_.pop();

    if (b->isAdd()) {
      imm_ = i1 + i2;
    } else if (b->isMultiply()) {
      imm_ = i1 * i2;
    } else {
      assert(0);
    }
    stack_.push(imm_);
  }
  virtual void visit(Immediate* i) {
    Result res = i->eval();
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
    stack_.push(imm_);
  }
  virtual void visit(Dereference* d) {
    // Don't dereference for now
    // Should do it in runtime, not in instrumentation time
  }

  Dyninst::Address imm() const {
    return imm_;
  }

private:
  std::stack<Dyninst::Address> stack_;
  Dyninst::Address imm_;
  Dyninst::Address a_;
};

/* We emulate the instruction by this sequence:
   Case 1: if R8 is not used in this instruction
    push %r8
    mov IMM, %r8
    modified original instruction, and use %r8
    pop %r8

   Case 2: if R8 is used in this instruciton
    push %r9
    mov IMM, %r9, and use %r9
    modified original instruction
    pop %r9
 */
// Emulate a PC-sensitive instruction
static size_t emulate_pcsen(Instruction::Ptr insn, Expression::Ptr e,
                            Dyninst::Address a, char* buf) {
  char* p = buf;
  char* insn_buf = (char*)insn->ptr();

  //--------------------------------------------------------
  // Step 1: see if %r8 is used, so get register first
  //--------------------------------------------------------
  // Get REX prefix, if it has one
  char rex = 0;
  int modrm_offset = 1;
  if ((insn_buf[0] & 0xf0) == 0x40) {
    //sp_debug("GOT REX prefix - %x", insn_buf[0]);
    rex = insn_buf[0];
    ++modrm_offset;
  }

  char escape = 0;
  if (insn_buf[modrm_offset-1] == 0x0f) {
    //sp_debug("GOT 0x0f");
    escape = 0x0f;
    ++modrm_offset;
  }

  // Get ModRM
  char modrm = insn_buf[modrm_offset];

  // Get the register used
  char reg = 0;
  if (rex) {
    if (rex & 0x04) reg |= 0x08; // 64-bit reg
  }
  reg |= ((modrm & 0x38) >> 3);

  //--------------------------------------------------------
  // Step 2: push %r8 | push %r9
  //--------------------------------------------------------
  // Push
  if (reg != 0x08) {
    // Can use R8
    *p++ = 0x41; // push %r8
    *p++ = 0x50;
  } else {
    // Use R9
    *p++ = 0x41; // push %r9
    *p++ = 0x51;
  }

  //--------------------------------------------------------
  // Step 3: mov imm, %r8 | mov imm, %r9
  //--------------------------------------------------------
  // Mov IMM64, %REG
  *p++ = 0x49;
  if (reg != 0x08) {
    *p++ = 0xb8; // mov imm64, %r8
  } else {
    *p++ = 0xb9; // mov imm64, %r9
  }
  long* l = (long*)p;
  // Get IMM64
  EmuVisitor visitor(a+insn->size());
  //for (set<Expression::Ptr>::iterator i = e.begin(); i != e.end(); i++) {
  //  (*i)->apply(&visitor);
  e->apply(&visitor);
  //  }
  *l = visitor.imm();
  p += sizeof(l);

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
  char new_modrm = modrm;
  if (reg != 0x08) {
    new_modrm &= 0xf8; // (R8), the last 3-bit should be 000
  } else {
    new_modrm &= 0xf9; // (R9), the last 3-bit should be 001
  }
  *p++ = new_modrm;
  // Copy imm after displacement
  for (int i = modrm_offset+1+4; i < insn->size(); i++) {
    *p++ = insn_buf[i];
  }
  //--------------------------------------------------------
  // Step 4: pop %r8 | pop %r9
  //--------------------------------------------------------
  // Pop
  if (reg != 0x08) {
    // Can use RAX
    *p++ = 0x41; // pop %r8
    *p++ = 0x58;
  } else {
    // Use RBX
    *p++ = 0x41; // pop %rbx
    *p++ = 0x59;
  }

  //sp_debug("REX: %x, ModRM: %x, Reg: %x", rex, modrm, reg);
  return (size_t)(p - buf);
}

static size_t reloc_insn_internal(Dyninst::Address a,
                                  Instruction::Ptr insn,
                                  std::set<Expression::Ptr>& exp,
                                  bool use_pc,
                                  char* p) {
  if (use_pc) {
    // Deal with PC-sensitive instruction
    char insn_buf[20];
    memcpy(insn_buf, insn->ptr(), insn->size());
    int* dis_buf = get_disp(insn, insn_buf);
    long old_rip = a;
    long new_rip = (long)p;
    long old_dis = *dis_buf;
    long long_new_dis = (old_rip - new_rip) + *dis_buf;
    //sp_debug("old_rip: %lx, new_rip: %lx, old_dis: %lx, new_dis: %lx, %d",
    //old_rip, new_rip, old_dis, long_new_dis, long_new_dis);

    if (sp::is_disp32(long_new_dis)) {
      // Easy case: just modify the displacement
      *dis_buf = (int)long_new_dis;
      memcpy(p, insn_buf, insn->size());
      return insn->size();
    } else {
      // General purpose: emulate the instruction
      size_t insn_size = emulate_pcsen(insn, *exp.begin(), a, p);
      /*
      sp_debug("DUMP EMULATE INSN (%d bytes) - {", insn_size);
      sp_debug("%s", context_->parser()->dump_insn((void*)p, insn_size).c_str());
      sp_debug("DUMP EMULATE INSN - }");*/
      return insn_size;
    }
  } else {
    // For non-pc-sensitive and non-last instruction, just copy it
    memcpy(p, insn->ptr(), insn->size());
    return insn->size();
  }
}

// Relocate an ordinary instruction
size_t SpSnippet::reloc_insn(Dyninst::Address src_insn,
                             Instruction::Ptr insn,
                             Dyninst::Address last,
                             char* buf) {
  // We don't handle last instruction for now
  if (src_insn == last) {  return 0;  }

  // See if this instruction is a pc-sensitive instruction
  set<Expression::Ptr> opSet;
  if (insn->readsMemory()) insn->getMemoryReadOperands(opSet);
  else if (insn->writesMemory()) insn->getMemoryWriteOperands(opSet);

  bool use_pc = false;
  for (set<Expression::Ptr>::iterator i = opSet.begin(); i != opSet.end(); i++) {
    RelocVisitor visitor(context_->parser());
    (*i)->apply(&visitor);
    use_pc = visitor.use_pc();
  }

  // Here we go!
  return reloc_insn_internal(src_insn, insn, opSet, use_pc, buf);
}


size_t SpSnippet::jump_abs_size() {
  // push x 4
  // ret
  return 17;
}

// Relocate the call instruction
// This is used in deadling with indirect call
size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset) {
  char* p = buf + offset;
  bool use_pc = false;

  // Check whether the call instruction uses RIP
  Instruction::Ptr insn = get_orig_call_insn();
  RelocVisitor visitor(context_->parser());
  Expression::Ptr trg = insn->getControlFlowTarget();
  set<Expression::Ptr> opSet;
  if (trg) {
    trg->apply(&visitor);
    use_pc = visitor.use_pc();
    opSet.insert(trg);
  }
  return reloc_insn_internal(src, insn, opSet, use_pc, p);
}

void* SpSnippet::pop_argument(ArgumentHandle* h, size_t size) {
  using namespace Dyninst::x86_64;
  if (h->num < 6) {
    Dyninst::Address a = 0;
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

long  SpSnippet::get_ret_val() {
  return get_saved_reg(Dyninst::x86_64::rax);
}

}
