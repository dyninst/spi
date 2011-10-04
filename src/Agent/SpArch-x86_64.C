#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

using Dyninst::PatchAPI::PatchFunction;

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
size_t SpSnippet::emit_save(char* buf, size_t offset) {
  char* p = buf + offset;
  /*
  *p++ = 0x54; // push rsp
  *p++ = 0x9c; // pushfq
  */

  *p++ = 0x57; // push rdi
  *p++ = 0x56; // push rsi
  *p++ = 0x52; // push rdx
  *p++ = 0x51; // push rcx
  *p++ = 0x41; // r8
  *p++ = 0x50;
  *p++ = 0x41; // r9
  *p++ = 0x51;
  *p++ = 0x41; // r10
  *p++ = 0x52;
  *p++ = 0x41; // r11
  *p++ = 0x53;
  *p++ = 0x50; // %rax

  /* Callee-saved: rbp, rbx, r12 ~ r15 */
  /*
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
  */
  return (p - (buf + offset));
}

size_t SpSnippet::emit_restore( char* buf, size_t offset) {
  char* p = buf + offset;
  /*
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
  */
  *p++ = 0x58; // rax
  *p++ = 0x41; // r11
  *p++ = 0x5b;
  *p++ = 0x41; // r10
  *p++ = 0x5a;
  *p++ = 0x41; // pop r9
  *p++ = 0x59;
  *p++ = 0x41; // pop r8
  *p++ = 0x58;
  *p++ = 0x59; // pop rcx
  *p++ = 0x5a; // pop rdx
  *p++ = 0x5e; // pop rsi
  *p++ = 0x5f; // pop rdi
  /*
  *p++ = 0x9d; // popfq
  *p++ = 0x5c; // pop rsp
  */
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

size_t SpSnippet::emit_pass_param(long point, long context,
                                  char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point, p, 0);
  p += insnsize;

  // movq SP_CONTEXT, %rsi
  insnsize = emit_mov_imm64_rsi((long)context, p, 0);
  p += insnsize;

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

size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;
  long retaddr = (long)p + 16 + 16 + 1;
  // push return address
  insnsize = emit_push_imm64(retaddr, p, 0);
  p += insnsize;
  // push callee address
  insnsize = emit_push_imm64(callee, p, 0);
  p += insnsize;
  // ret
  //*p = 0xcb;
  *p = 0xc3;
  p ++;
  assert(retaddr == (long)p);

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

size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push jump target
  insnsize = emit_push_imm64(trg, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;
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

Dyninst::Address SpParser::get_saved_reg(Dyninst::MachRegister reg) {
  sp_debug("INDIRECT - get saved register %s", reg.name().c_str());
  // sp_print("call_addr in %s", reg.name().c_str());

  using namespace Dyninst::x86_64;
  mcontext_t* c = &old_context_.uc_mcontext;

  if (reg == eax) {
    return c->gregs[REG_RAX];
  } else if (reg == ebx) {
    return c->gregs[REG_RBX];
  } else if (reg == ecx) {
    return c->gregs[REG_RCX];
  } else if (reg == edx) {
    return c->gregs[REG_RDX];
  } else if (reg == esp) {
    return c->gregs[REG_RSP];
  } else if (reg == rax) {
    return c->gregs[REG_RAX];
  } else if (reg == rbx) {
    return c->gregs[REG_RBX];
  } else if (reg == rcx) {
    return c->gregs[REG_RCX];
  } else if (reg == rdx) {
    return c->gregs[REG_RDX];
  } else if (reg == rsp) {
    return c->gregs[REG_RSP];
  } else if (reg == r8) {
    return c->gregs[REG_R8];
  } else if (reg == r9) {
    return c->gregs[REG_R9];
  } else if (reg == r10) {
    return c->gregs[REG_R10];
  } else if (reg == r11) {
    return c->gregs[REG_R11];
  } else if (reg == r12) {
    return c->gregs[REG_R12];
  } else if (reg == r13) {
    return c->gregs[REG_R13];
  } else if (reg == r14) {
    return c->gregs[REG_R14];
  } else if (reg == r15) {
    return c->gregs[REG_R15];
  } else {
    assert(0);
  }
}

}
