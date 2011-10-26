#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

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
  *p++ = 0x50;  // push %rax
  *p++ = 0x48;  // mov loc, %rax
  *p++ = 0xb8;
  long* l = (long*)p;
  *l = loc;
  p += sizeof(long);
  *p++ = 0x48;  // mov %rsp, (%rax)
  *p++ = 0x89;
  *p++ = 0x20;
  *p++ = 0x58;  // pop %rax

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
  Dyninst::Address rel_addr_abs = (callee > retaddr) ?
    (callee - retaddr) : (retaddr - callee);

  if (rel_addr_abs <= 0x7fffffff) {
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
  Dyninst::Address rel_addr_abs = (trg > retaddr) ?
    (trg - retaddr) : (retaddr - trg);
  if (rel_addr_abs <= 0x7fffffff && !abs) {
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
                                         size_t orig_insn_size) {
  sp_debug("INDIRECT - get saved register %s", reg.name().c_str());
  sp_debug("SAVED CONTEXT - at %lx", sp);
  // sp_print("call_addr in %s", reg.name().c_str());

  const int RAX = 0;
  const int R9 = 8;
  const int R8 = 16;
  const int RCX = 24;
  const int RDX = 32;
  const int RSI = 40;
  const int RDI = 48;
  const int RBP = 56;
  const int R15 = 64;
  const int R14 = 72;
  const int R13 = 80;
  const int R12 = 88;
  const int RBX = 96;
  const int R11 = 104;
  const int R10 = 112;
  const int RSP = 120;

#define reg_val(i) (*(long*)(sp+(i)))

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
  if (r == Dyninst::x86_64::rip) return true;
  return false;
}

size_t SpSnippet::reloc_insn(Dyninst::PatchAPI::PatchBlock::Insns::iterator i,
                             Dyninst::Address last,
                             char* buf) {
  using namespace Dyninst::InstructionAPI;
  char* p = buf;
  Dyninst::Address a = i->first;
  Instruction::Ptr insn = i->second;

  if (a == last) {
    return 0;
  } else {
    memcpy(p, insn->ptr(), insn->size());
    return insn->size();
  }
}

}
