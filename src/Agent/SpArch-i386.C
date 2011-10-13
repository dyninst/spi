#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

namespace sp {

// dump context
void SpSnippet::dump_context(ucontext_t* context) {
  if (!context) {
    return;
  }
  mcontext_t* c = &context->uc_mcontext;
  sp_debug("DUMP CONTEXT - {");

  sp_debug("eax: %lx", c->gregs[REG_EAX]);
  sp_debug("ebx: %lx", c->gregs[REG_EBX]);
  sp_debug("ecx: %lx", c->gregs[REG_ECX]);
  sp_debug("edx: %lx", c->gregs[REG_EDX]);
  sp_debug("esi: %lx", c->gregs[REG_ESI]);
  sp_debug("edi: %lx", c->gregs[REG_EDI]);
  sp_debug("ebp: %lx", c->gregs[REG_EBP]);
  sp_debug("esp: %lx", c->gregs[REG_ESP]);
  sp_debug("flags: %lx", c->gregs[REG_EFL]);

  sp_debug("DUMP CONTEXT - }");
}

// a bunch of code generation functions
size_t SpSnippet::emit_save(char* buf, size_t offset) {
  char* p = buf + offset;

  //  *p++ = 0x54; // push esp
  //  *p++ = 0x9c; // pushf
  /*
  *p++ = 0x57; // push edi
  *p++ = 0x56; // push esi
  *p++ = 0x52; // push edx
  *p++ = 0x51; // push ecx
  *p++ = 0x53; // push ebx
  *p++ = 0x50; // push eax
*/
  *p++ = 0x60; // pusha
  return (p - (buf + offset));
}

size_t SpSnippet::emit_restore( char* buf, size_t offset) {
  char* p = buf + offset;
  *p++ = 0x61; // popa

  /*
  *p++ = 0x58; // pop eax
  *p++ = 0x5b; // pop ebx
  *p++ = 0x59; // pop ecx
  *p++ = 0x5a; // pop edx
  *p++ = 0x5e; // pop esi
  *p++ = 0x5f; // pop edi
  */
  //  *p++ = 0x9d; // popf
  //  *p++ = 0x5c; // pop esp

  return (p - (buf + offset));
}

// for debug, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
  char* p = buf + offset;
  // mov 0, 0

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

static size_t emit_push_imm32(long imm, char* buf, size_t offset) {
  char* p = buf + offset;

  *p++ = 0x68;
  long* i = (long*)p;
  *i = imm;
  p += sizeof(long);

  return (p - (buf + offset));
}

static size_t emit_pop_imm32(char* buf, size_t offset) {
  char* p = buf + offset;

  // add $0x4, %esp
  /*
  *p++ = 0x83;
  *p++ = 0xc4;
  *p++ = 0x04;
  */
  *p++ = 0x58;  // pop eax
  return (p - (buf + offset));
}

size_t SpSnippet::emit_pass_param(long point, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push POINT
  insnsize = emit_push_imm32((long)point, p, 0);
  p += insnsize;

  return (p - (buf + offset));
}


size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset, bool restore) {
  /*
  char* p = buf + offset;
  size_t insnsize = 0;
  long retaddr = (long)p + 5 + 5 + 1;

  // push return address
  insnsize = emit_push_imm32(retaddr, p, 0);
  p += insnsize;
  // push callee address
  insnsize = emit_push_imm32(callee, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;
  assert(retaddr == (long)p);
  */
  char* p = buf + offset;
  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  size_t insnsize = 0;
  Dyninst::Address rel_addr = (callee - retaddr);
  Dyninst::Address rel_addr_abs = (callee > retaddr) ?
    (callee - retaddr) : (retaddr - callee);

  if (rel_addr_abs <= 0xffffffff) {
    *p++ = 0xe8;
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += 4;
  } else {
    sp_perror("larger than 4 bytes for call address");
  }

  if (restore) {
    // pop param
    insnsize = emit_pop_imm32(p, 0);
    p += insnsize;
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
  /*
  insnsize = emit_push_imm64(callee, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;
*/
  return (p - (buf + offset));
}

size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;
  /*
  // push jump target
  insnsize = emit_push_imm32(trg, p, 0);
  p += insnsize;

  // ret
  *p++ = 0xc3;
  */

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
    insnsize = emit_push_imm32(trg, p, 0);
    p += insnsize;
    // ret
    *p++ = 0xc3;
  }

  return (p - (buf + offset));
}


Dyninst::Address SpSnippet::get_pre_signal_pc(void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_EIP];
}

Dyninst::Address SpSnippet::set_pc(Dyninst::Address pc, void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_EIP] = pc;
}

Dyninst::Address SpParser::get_saved_reg(Dyninst::MachRegister reg,
                                         size_t orig_insn_size) {
  sp_debug("INDIRECT - get saved register %s", reg.name().c_str());
  // sp_print("call_addr in %s", reg.name().c_str());

  using namespace Dyninst::x86;
  mcontext_t* c = &old_context_.uc_mcontext;

  if (reg == eax) {
    return c->gregs[REG_EAX];
  } else if (reg == ebx) {
    return c->gregs[REG_EBX];
  } else if (reg == ecx) {
    return c->gregs[REG_ECX];
  } else if (reg == edx) {
    return c->gregs[REG_EDX];
  } else if (reg == esp) {
    return c->gregs[REG_ESP];
  } else if (reg == esi) {
    return c->gregs[REG_ESI];
  } else if (reg == edi) {
    return c->gregs[REG_EDI];
  } else if (reg == ebp) {
    return c->gregs[REG_EBP];
  } else if (reg == eip) {
    return c->gregs[REG_EIP]-1+orig_insn_size;
  } else if (reg == esi) {
    return c->gregs[REG_ESI];
  } else if (reg == edi) {
    return c->gregs[REG_EDI];
  } else {
    sp_print("get saved register %s", reg.name().c_str());
    assert(0);
  }

}

bool SpParser::is_pc(Dyninst::MachRegister r) {
  if (r == Dyninst::x86::eip) return true;
  return false;
}


}
