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
size_t SpSnippet::emit_save(char* buf, size_t offset, bool) {
  char* p = buf + offset;
  *p++ = 0x60; // pusha
  return (p - (buf + offset));
}

size_t SpSnippet::emit_restore( char* buf, size_t offset, bool) {
  char* p = buf + offset;
  *p++ = 0x61; // popa
  return (p - (buf + offset));
}

// for debug, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
  char* p = buf + offset;

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

size_t SpSnippet::emit_save_sp(long loc, char* buf, size_t offset) {
  char* p = buf + offset;
  // mov %esp, (loc)
  *p++ = 0x89;
  *p++ = 0x25;

  long* l = (long*)p;
  *l = loc;
  p += sizeof(long);

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
  *p++ = 0x58;  // pop eax
/*
  *p++ = 0x83;  // add $0x4, esp
  *p++ = 0xc4;
  *p++ = 0x04;
  */
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
  insnsize = emit_push_imm32(callee, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;

  return (p - (buf + offset));
}

size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset, bool abs) {
  char* p = buf + offset;
  size_t insnsize = 0;

  sp_debug("EMIT JUMP START");

  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  Dyninst::Address rel_addr = (trg - retaddr);
  Dyninst::Address rel_addr_abs = (trg > retaddr) ?
    (trg - retaddr) : (retaddr - trg);
  if (rel_addr_abs <= 0x7fffffff && !abs) {
    sp_debug("REL JUMP START");

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
                                         Dyninst::Address sp,
                                         size_t offset) {
  sp_debug("INDIRECT - get saved register %s", reg.name().c_str());
  sp_debug("SAVED CONTEXT - at %lx", sp);
  /* Push(EAX); Push(ECX); Push(EDX); Push(EBX); Push(Temp); Push(EBP); Push(ESI); Push(EDI); */

  const int EDI = 0+offset;
  const int ESI = 4+offset;
  const int EBP = 8+offset;
  const int ESP = 12+offset;
  const int EBX = 16+offset;
  const int EDX = 20+offset;
  const int ECX = 24+offset;
  const int EAX = 28+offset;

#define reg_val(i) (*(long*)(sp+(i)))

  for (int i = 0; i < 32; i+=4) {
    sp_debug("i: %d, EDI: %lx", i, reg_val(i));
  }

  using namespace Dyninst::x86;
  if (reg == edi) return reg_val(EDI);
  if (reg == esi) return reg_val(ESI);
  if (reg == ebp) return reg_val(EBP);
  if (reg == esp) return reg_val(ESP);
  if (reg == ebx) return reg_val(EBX);
  if (reg == edx) return reg_val(EDX);
  if (reg == ecx) return reg_val(ECX);
  if (reg == eax) return reg_val(EAX);
  sp_debug("NO FOUND");
  return 0;
}

bool SpParser::is_pc(Dyninst::MachRegister r) {
  if (r == Dyninst::x86::eip) return true;
  return false;
}

size_t SpSnippet::jump_abs_size() {
  // pushl imm;
  // ret
  return 7;
}

/* For i386
 if (call insn && ! last insn) {
   adjust thunk relative addr
   relocate
 } else if (last insn) {
   skip
 } else {
   relocate
 }
  */
size_t SpSnippet::reloc_insn(Dyninst::PatchAPI::PatchBlock::Insns::iterator i,
                             Dyninst::Address last,
                             char* buf) {
  using namespace Dyninst::InstructionAPI;
  char* p = buf;
  Dyninst::Address a = i->first;
  Instruction::Ptr insn = i->second;
  if (insn->getCategory() == Dyninst::InstructionAPI::c_CallInsn &&
      a != last)  {

    // What thunk does, is to move current pc value to ebx.
    // mov orig_pc, ebx
    *p++ = 0xbb;
    long* new_ebx = (long*)p;
    *new_ebx = (long)(a+5);
    return 5;
  } else if (a == last) {
    return 0;
  } else {
    memcpy(p, insn->ptr(), insn->size());
    return insn->size();
  }
}

size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset,bool) {
  char* p = buf + offset;
  char* psrc = (char*)src;

  for (size_t i = 0; i < size; i++)
    *p++ = psrc[i];

  return (p - (buf + offset));
}

}
