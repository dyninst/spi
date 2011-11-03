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

// dump context
void SpSnippet::dump_context(ucontext_t* context) {
  if (!context) {
    return;
  }
  mcontext_t* c = &context->uc_mcontext;
  sp_debug("DUMP CONTEXT - {");

  sp_debug("rax: %lx", c->gregs[REG_RAX]);
  sp_debug("rbx: %lx", c->gregs[REG_RBX]);
  sp_debug("rcx: %lx", c->gregs[REG_RCX]);
  sp_debug("rdx: %lx", c->gregs[REG_RDX]);
  sp_debug("rsi: %lx", c->gregs[REG_RSI]);
  sp_debug("rdi: %lx", c->gregs[REG_RDI]);
  sp_debug("rbp: %lx", c->gregs[REG_RBP]);
  sp_debug("rsp: %lx", c->gregs[REG_RSP]);
  sp_debug("r8: %lx", c->gregs[REG_R8]);
  sp_debug("r9: %lx", c->gregs[REG_R9]);
  sp_debug("r10: %lx", c->gregs[REG_R10]);
  sp_debug("r11: %lx", c->gregs[REG_R11]);
  sp_debug("r12: %lx", c->gregs[REG_R12]);
  sp_debug("r13: %lx", c->gregs[REG_R13]);
  sp_debug("r14: %lx", c->gregs[REG_R14]);
  sp_debug("r15: %lx", c->gregs[REG_R15]);
  sp_debug("flags: %lx", c->gregs[REG_EFL]);

  sp_debug("DUMP CONTEXT - }");
}

// a bunch of code generation functions
size_t SpSnippet::emit_save(char* buf, size_t offset, bool indirect) {
  char* p = buf + offset;

  // Saved for indirect call
  if (indirect) {
    *p++ = 0x54; // push rsp
    *p++ = 0x41; // r10 unused in C
    *p++ = 0x52;
    *p++ = 0x41; // r11 for linker
    *p++ = 0x53;
    *p++ = 0x53; // %rbx
    *p++ = 0x41; // r12
    *p++ = 0x54;
    *p++ = 0x41; // r13
    *p++ = 0x55;
    *p++ = 0x41; // r14
    *p++ = 0x56;
    *p++ = 0x41; // r15
    *p++ = 0x57;
    *p++ = 0x55; // rbp
  }

  // Saved for direct/indirect call
  *p++ = 0x57; // push rdi
  *p++ = 0x56; // push rsi
  *p++ = 0x52; // push rdx
  *p++ = 0x51; // push rcx
  *p++ = 0x41; // r8
  *p++ = 0x50;
  *p++ = 0x41; // r9
  *p++ = 0x51;
  *p++ = 0x50; // %rax


  return (p - (buf + offset));
}

size_t SpSnippet::emit_restore( char* buf, size_t offset, bool indirect) {
  char* p = buf + offset;

  // Restored for direct/indirect call
  *p++ = 0x58; // rax
  *p++ = 0x41; // pop r9
  *p++ = 0x59;
  *p++ = 0x41; // pop r8
  *p++ = 0x58;
  *p++ = 0x59; // pop rcx
  *p++ = 0x5a; // pop rdx
  *p++ = 0x5e; // pop rsi
  *p++ = 0x5f; // pop rdi

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
    *p++ = 0x5c; // pop rsp
  }
  return (p - (buf + offset));
}

size_t SpSnippet::emit_save_sp(long loc, char* buf, size_t offset) {
  char* p = buf + offset;
  /*
  400448:       50                      push   %rax
  400449:       48 b8 56 34 12 90 78    mov    $0x1234567890123456,%rax
  400450:       56 34 12 
  400453:       48 89 20                mov    %rsp,(%rax)
  400456:       58                      pop    %rax
   */
  //*p++ = 0x50;  // push %rax
  *p++ = 0x48;  // mov loc, %rax
  *p++ = 0xb8;
  long* l = (long*)p;
  *l = loc;
  p += sizeof(long);
  *p++ = 0x48;  // mov %rsp, (%rax)
  *p++ = 0x89;
  *p++ = 0x20;
  //*p++ = 0x58;  // pop %rax

  return (p - (buf + offset));
}

// for debug, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
  char* p = buf + offset;
  // mov 0, 0
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

static size_t emit_mov_imm64_rdi(long imm, char* buf, size_t offset) {
  char* p = buf + offset;
  *p = (char)0x48; p++;
  *p = (char)0xbf; p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

static size_t emit_mov_imm64_rsi(long imm, char* buf, size_t offset) {
  char* p = buf + offset;
  *p = (char)0x48; p++;
  *p = (char)0xbe; p++;
  *((long*)p) = (long)imm;
  return (2 + sizeof(long));
}

size_t SpSnippet::emit_pass_param(long point, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point, p, 0);
  p += insnsize;

  // movq SP_CONTEXT, %rsi
  //insnsize = emit_mov_imm64_rsi((long)g_context, p, 0);
  //p += insnsize;

  return (p - (buf + offset));
}

static size_t emit_push_imm64(long imm, char* buf, size_t offset) {
  char* p = buf + offset;

  for (int i = 3; i >= 0; i--) {
    short word = static_cast<unsigned short>((imm >> (16 * i)) & 0xffff);
    *p++ = 0x66; // operand size override
    *p++ = 0x68; // push immediate (16-bits b/c of prefix)
    *(short *)p = word;
    p += 2;
  }
  return (p - (buf + offset));
}

size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset, bool) {
  char* p = buf + offset;
  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  size_t insnsize = 0;
  Dyninst::Address rel_addr = (callee - retaddr);

  if (sp::is_disp32(rel_addr)) {
    *p++ = 0xe8;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    *p++ = 0x48; // movq call_addr, %rax
    *p++ = 0xb8;
    long* call_addr = (long*)p;
    *call_addr = callee;
    p += sizeof(long);

    *p++ = 0xff; // call %rax
    *p++ = 0xd0;

    //sp_debug("SHIFT OFFSET BY 4");
    //context_->parser()->set_sp_offset(sizeof(long));
  }

  return (p - (buf + offset));
}

size_t SpSnippet::emit_ret(char* buf, size_t offset) {
  char* p = buf + offset;
  *p++ = 0xc3;
  return (p - (buf + offset));
}

// if the original call is performed by jump instruction (tail call optimization)
// then we don't push the return address
size_t SpSnippet::emit_call_jump(long callee, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push callee address
  insnsize = emit_push_imm64(callee, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;

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


Dyninst::Address SpSnippet::get_pre_signal_pc(void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_RIP];
}

Dyninst::Address SpSnippet::set_pc(Dyninst::Address pc, void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_RIP] = pc;
}

Dyninst::Address SpParser::get_saved_reg(Dyninst::MachRegister reg,
                                         Dyninst::Address sp,
                                         size_t offset) {
  sp_debug("INDIRECT - get saved register %s", reg.name().c_str());
  sp_debug("SAVED CONTEXT - at %lx", sp);
  // sp_print("call_addr in %s", reg.name().c_str());

  const int RAX = 0+offset;
  const int R9  = 8+offset;
  const int R8  = 16+offset;
  const int RCX = 24+offset;
  const int RDX = 32+offset;
  const int RSI = 40+offset;
  const int RDI = 48+offset;
  const int RBP = 56+offset;
  const int R15 = 64+offset;
  const int R14 = 72+offset;
  const int R13 = 80+offset;
  const int R12 = 88+offset;
  const int RBX = 96+offset;
  const int R11 = 104+offset;
  const int R10 = 112+offset;
  const int RSP = 120+offset;

#define reg_val(i) (*(long*)(sp+(i)))

  for (int i = 0; i < 16; i++) {
    sp_debug("DUMP SAVED REGS: %lx", reg_val((i*8)));
  }

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
  if (reg == rsp) return reg_val(RSP);
  if (reg == rbp) return reg_val(RBP);

  if (reg == eax) return reg_val(RAX);
  if (reg == ebx) return reg_val(RBX);
  if (reg == ecx) return reg_val(RCX);
  if (reg == edx) return reg_val(RDX);
  if (reg == esi) return reg_val(RSI);
  if (reg == edi) return reg_val(RDI);
  if (reg == esp) return reg_val(RSP);
  if (reg == ebp) return reg_val(RBP);

  sp_print("Cannot find register %s", reg.name().c_str());
  return 0;
}

bool SpParser::is_pc(Dyninst::MachRegister r) {
  sp_debug("IS PC ? - %s", r.name().c_str());
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

  /*
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
using namespace Dyninst::InstructionAPI;
static int* get_disp(Instruction::Ptr insn, char* insn_buf) {
  int* disp = NULL;

  int disp_offset = 0;
  // Any REX?
  if ((insn_buf[disp_offset] & 0xf0) == 0x40) {
    sp_debug("GOT REX prefix - %x", insn_buf[disp_offset]);
    ++disp_offset;
  }

  // Any ESCAPE?
  if (insn_buf[disp_offset] == 0x0f) {
    sp_debug("GOT REX prefix - %x", insn_buf[disp_offset]);
    ++disp_offset;
  }

  // OPCODE
  ++disp_offset;

  // MODRM
  ++disp_offset;

  disp = (int*)&insn_buf[disp_offset];
  return disp;
}

class EmuVisitor : public Visitor {
public:
  EmuVisitor(Dyninst::Address a)
    : Visitor(), imm_(0), a_(a) { }
  virtual void visit(RegisterAST* r) {
    imm_ = a_;
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
   Case 1: if RAX is not used in this instruction
    push %r8
    mov IMM, %r8
    modified original instruction
    pop %r8

   Case 2: if RAX is used in this instruciton
    push %r9
    mov IMM, %r9
    modified original instruction
    pop %r9
 */
static size_t emulate_pcsen(Instruction::Ptr insn, set<Expression::Ptr>& e,
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
    sp_debug("GOT REX prefix - %x", insn_buf[0]);
    rex = insn_buf[0];
    ++modrm_offset;
  }

  char escape = 0;
  if (insn_buf[modrm_offset-1] == 0x0f) {
    sp_debug("GOT 0x0f");
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
  for (set<Expression::Ptr>::iterator i = e.begin(); i != e.end(); i++) {
    (*i)->apply(&visitor);
  }
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

  sp_debug("REX: %x, ModRM: %x, Reg: %x", rex, modrm, reg);
  return (size_t)(p - buf);
}

size_t SpSnippet::reloc_insn(Dyninst::PatchAPI::PatchBlock::Insns::iterator i,
                             Dyninst::Address last,
                             char* buf) {
  char* p = buf;
  Dyninst::Address a = i->first;
  Instruction::Ptr insn = i->second;

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

  if (a == last) {
    // We don't handle last instruction for now
    return 0;
  } else if (use_pc) {
    // Deal with PC-sensitive instruction
    //REMOVEME {
    //return 0;
    //}

    sp_debug("USE PC");
    char insn_buf[20];
    bool use_rax = false;
    memcpy(insn_buf, insn->ptr(), insn->size());
    int* dis_buf = get_disp(insn, insn_buf);

    long old_rip = a;
    long new_rip = (long)p;
    long old_dis = *dis_buf;
    long long_new_dis = (old_rip - new_rip) + *dis_buf;
    sp_debug("old_rip: %lx, new_rip: %lx, old_dis: %lx, new_dis: %lx, %d",
             old_rip, new_rip, old_dis, long_new_dis, long_new_dis);

    // REMOVEME{
    /*
    if (!sp::is_disp32(long_new_dis)) {
      sp_debug("SHORTCUT, REMOVE FOR NOW");
      size_t insn_size = emulate_pcsen(insn, opSet, a, p);
      sp_debug("ORIGINAL %s", context_->parser()->dump_insn((void*)insn_buf, insn->size()).c_str());
      sp_debug("DUMP REVERSE INSN (%d bytes) - {", insn_size);
      sp_debug("%s", context_->parser()->dump_insn((void*)p, insn_size).c_str());
      sp_debug("DUMP REVERSE INSN - }");
      return -1;
      // }
      */
    if (sp::is_disp32(long_new_dis)) {
      // Easy case: just modify the displacement
      *dis_buf = (int)long_new_dis;
      memcpy(p, insn_buf, insn->size());
      return insn->size();
    } else {
      // REMOVEME {
      //      return -1;
      // }
      // General purpose: emulate the instruction
      size_t insn_size = emulate_pcsen(insn, opSet, a, p);
      sp_debug("DUMP EMULATE INSN (%d bytes) - {", insn_size);
      sp_debug("%s", context_->parser()->dump_insn((void*)p, insn_size).c_str());
      sp_debug("DUMP EMULATE INSN - }");
      return insn_size;
    }
  } else {
    // For non-pc-sensitive and non-last instruction, just copy it
    memcpy(p, insn->ptr(), insn->size());
    return insn->size();
  }
}

size_t SpSnippet::jump_abs_size() {
  // push x 4
  // ret
  return 17;
}

class CallInsnVisitor : public Visitor {
  public:
    CallInsnVisitor(SpParser::ptr p)
      : Visitor(), p_(p), use_pc_(false) {}
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

size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset,
                                 bool tail) {
  char* p = buf + offset;
  char* psrc = (char*)src;
  bool use_pc = false;

  // Check whether the call instruction uses RIP
  Dyninst::PatchAPI::PatchBlock* blk = point_->block();
  //Instruction::Ptr insn = blk->getInsn(blk->last());
  Instruction::Ptr insn = get_orig_call_insn();
  CallInsnVisitor visitor(context_->parser());
  Expression::Ptr trg = insn->getControlFlowTarget();
  if (trg) {
    trg->apply(&visitor);
    use_pc = visitor.use_pc();
  }

  if (!use_pc) {
    sp_debug("Emit orig call");
    // If not using RIP, then copy the instruction
    for (size_t i = 0; i < size; i++)
      *p++ = psrc[i];
  } else {
    // assert(0 && "RIP-sensitive call");
    //REMOVEME {
    //return 0;
    //}
    sp_debug("Emit RIP call");
    before_call_orig_ = offset;
    memset(p, 0x90, 12);
    p += 12; // reserve 12 bytes for future fixup
  }
  return (p - (buf + offset));
}

}
