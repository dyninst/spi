#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "SpUtils.h"
#include "SpPoint.h"

namespace sp {

//==============================================================================
// Code generation
//==============================================================================

// Save context before calling payload
size_t SpSnippet::emit_save(char* buf, size_t offset, bool) {
  char* p = buf + offset;
  *p++ = 0x60; // pusha

  sp::SpPoint* spt = static_cast<sp::SpPoint*>(point_);
  long* l = spt->saved_context_ptr();
  size_t insnsize = emit_save_sp((long)l, p, 0);
  p += insnsize;

  return (p - (buf + offset));
}

// Restore context after calling payload
size_t SpSnippet::emit_restore( char* buf, size_t offset, bool) {
  char* p = buf + offset;
  *p++ = 0x61; // popa
  return (p - (buf + offset));
}

// For debugging, cause segment fault
size_t SpSnippet::emit_fault(char* buf, size_t offset) {
  char* p = buf + offset;

  *p++ = (char)0xc7;  // movl $0x0, 0
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
size_t SpSnippet::emit_save_sp(long loc, char* buf, size_t offset) {
  char* p = buf + offset;

  *p++ = 0x89;   // mov %esp, (loc)
  *p++ = 0x25;

  long* l = (long*)p;
  *l = loc;
  p += sizeof(long);

  return (p - (buf + offset));
}

// Save an imm32 in stack
static size_t emit_push_imm32(long imm, char* buf, size_t offset) {
  char* p = buf + offset;

  *p++ = 0x68;        // push imm
  long* i = (long*)p;
  *i = imm;
  p += sizeof(long);

  return (p - (buf + offset));
}

// Get the saved imm32 from stack to %eax
static size_t emit_pop_imm32(char* buf, size_t offset) {
  char* p = buf + offset;
  *p++ = 0x58;  // pop eax
  return (p - (buf + offset));
}

// Pass parameter to payload function, which has only one parameter POINT
size_t SpSnippet::emit_pass_param(long point, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push POINT
  insnsize = emit_push_imm32((long)point, p, 0);
  p += insnsize;

  return (p - (buf + offset));
}

// Call a function w/ address `callee`
// `restore` indicates whether we need to pop out the pushed argument, this only
// happens when we call payload function
size_t SpSnippet::emit_call_abs(long callee, char* buf, size_t offset, bool restore) {
  char* p = buf + offset;
  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  size_t insnsize = 0;
  Dyninst::Address rel_addr = (callee - retaddr);

  if (sp::is_disp32(rel_addr)) {
    *p++ = 0xe8;    // call callee
    int* rel_p = (int*)p;
    *rel_p = rel_addr;
    p += sizeof(long);
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

// Jump to `trg`.
// `abs` indicates whether we must use absolute jump. If `abs` is false,
// we make such decision dynamically; otherwise, we mandate absolute jump for
// determinism.
size_t SpSnippet::emit_jump_abs(long trg, char* buf, size_t offset, bool abs) {
  char* p = buf + offset;
  size_t insnsize = 0;

  Dyninst::Address retaddr = (Dyninst::Address)p+5;
  Dyninst::Address rel_addr = (trg - retaddr);

  if (sp::is_disp32(rel_addr) && !abs) {
    *p++ = 0xe9;     // jmp trg
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

// Relocate the call instruction
// This is used in deadling with indirect call
size_t SpSnippet::emit_call_orig(long src, size_t size,
                                 char* buf, size_t offset) {
  char* p = buf + offset;
  char* psrc = (char*)src;

  memcpy(p, psrc, size);
  return size;
}

// Relocate an ordinary instruction
using namespace Dyninst::InstructionAPI;
size_t SpSnippet::reloc_insn(Dyninst::Address src_insn,
                             Instruction::Ptr insn,
                             Dyninst::Address last,
                             char* buf) {
  //----------------------------------------------
  // Case 1: we don't relocate the last insn
  //         because we'll do it later
  //----------------------------------------------
  Dyninst::Address a = src_insn;
  if (a == last)  return 0;


  char* p = buf;
  if (insn->getCategory() == c_CallInsn &&
      a != last)  {
    //-------------------------------------------------------
    // Case 2: handle thunk call
    // What thunk does, is to move current pc value to ebx.
    // mov orig_pc, ebx
    //-------------------------------------------------------
    *p++ = 0xbb;
    long* new_ebx = (long*)p;
    *new_ebx = (long)(a+5);
    return 5;
  } else {
    //-------------------------------------------------------
    // Case 3: other instructions
    //-------------------------------------------------------
    memcpy(p, insn->ptr(), insn->size());
    return insn->size();
  }
}

//==============================================================================
// Miscellaneous
//==============================================================================

// Used in trap handler to decide the pc value right at the call
Dyninst::Address SpSnippet::get_pre_signal_pc(void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_EIP];
}

// Used in trap handler to jump to snippet
Dyninst::Address SpSnippet::set_pc(Dyninst::Address pc, void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_EIP] = pc;
}

// Get the saved register, for resolving indirect call
Dyninst::Address SpParser::get_saved_reg(Dyninst::MachRegister reg,
                                         Dyninst::Address sp,
                                         size_t offset) {

  /* Pushed Left to right in order:
     Push(EAX); Push(ECX); Push(EDX); Push(EBX); Push(Temp); Push(EBP); Push(ESI); Push(EDI); */
  const int EDI = 0+offset;
  const int ESI = 4+offset;
  const int EBP = 8+offset;
  const int ESP = 12+offset;
  const int EBX = 16+offset;
  const int EDX = 20+offset;
  const int ECX = 24+offset;
  const int EAX = 28+offset;

#define reg_val(i) (*(long*)(sp+(i)))
  /*
  for (int i = 0; i < 32; i+=4) {
    sp_debug("i: %d, EDI: %lx", i, reg_val(i));
  }
  */
  using namespace Dyninst::x86;
  if (reg == edi) return reg_val(EDI);
  if (reg == esi) return reg_val(ESI);
  if (reg == ebp) return reg_val(EBP);
  if (reg == esp) return reg_val(ESP);
  if (reg == ebx) return reg_val(EBX);
  if (reg == edx) return reg_val(EDX);
  if (reg == ecx) return reg_val(ECX);
  if (reg == eax) return reg_val(EAX);

  return 0;
}

// Is this register EIP?
bool SpParser::is_pc(Dyninst::MachRegister r) {
  if (r == Dyninst::x86::eip) return true;
  return false;
}

// The size of an absolute jump in our code generation 
size_t SpSnippet::jump_abs_size() {
  // pushl imm;
  // ret
  return 7;
}

}
