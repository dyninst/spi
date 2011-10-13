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
size_t SpSnippet::emit_save(char* buf, size_t offset) {
  char* p = buf + offset;

  //*p++ = 0x54; // push rsp
  //*p++ = 0x9c; // pushfq

  *p++ = 0x57; // push rdi
  *p++ = 0x56; // push rsi
  *p++ = 0x52; // push rdx
  *p++ = 0x51; // push rcx
  *p++ = 0x41; // r8
  *p++ = 0x50;
  *p++ = 0x41; // r9
  *p++ = 0x51;
  //*p++ = 0x41; // r10 unused in C
  //*p++ = 0x52;
  //*p++ = 0x41; // r11 for linker
  //*p++ = 0x53;
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
  //*p++ = 0x41; // r11
  //*p++ = 0x5b;
  //*p++ = 0x41; // r10
  //*p++ = 0x5a;
  *p++ = 0x41; // pop r9
  *p++ = 0x59;
  *p++ = 0x41; // pop r8
  *p++ = 0x58;
  *p++ = 0x59; // pop rcx
  *p++ = 0x5a; // pop rdx
  *p++ = 0x5e; // pop rsi
  *p++ = 0x5f; // pop rdi

  //*p++ = 0x9d; // popfq
  //*p++ = 0x5c; // pop rsp

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

  // sp_print("%lx, %d", rel_addr_abs, (rel_addr_abs <= 0xffffffff));
  if (rel_addr_abs <= 0xffffffff) {
    *p++ = 0xe8;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    /*
    retaddr = (long)p + 16 + 16 + 1;
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
*/
    /*
    *p++ = 0x55; // push rbp

    *p++ = 0x48; // movq call_addr, %rbp
    *p++ = 0xbd;
    long* call_addr = (long*)p;
    *call_addr = callee;
    p += sizeof(long);

    *p++ = 0xff; // call %rbp
    *p++ = 0xd5;

    *p++ = 0x5d; // pop rbp
*/

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

size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  Dyninst::Address rel_addr = (trg - retaddr);
  Dyninst::Address rel_addr_abs = (trg > retaddr) ?
    (trg - retaddr) : (retaddr - trg);
  if (rel_addr_abs <= 0xffffffff) {
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
                                         size_t orig_insn_size) {
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
  } else if (reg == esi) {
    return c->gregs[REG_RSI];
  } else if (reg == edi) {
    return c->gregs[REG_RDI];
  } else if (reg == ebp) {
    return c->gregs[REG_RBP];
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
  } else if (reg == rip) {
    return c->gregs[REG_RIP]-1+orig_insn_size;
  } else if (reg == rsi) {
    return c->gregs[REG_RSI];
  } else if (reg == rdi) {
    return c->gregs[REG_RDI];
  } else if (reg == rbp) {
    return c->gregs[REG_RBP];
  } else {
    sp_print("get saved register %s", reg.name().c_str());
    assert(0);
  }
}

bool SpParser::is_pc(Dyninst::MachRegister r) {
  if (r == Dyninst::x86_64::rip) return true;
  return false;
}

}
