#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

using Dyninst::PatchAPI::PatchFunction;

namespace sp {

// dump context
void dump_context(ucontext_t* context) {
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
static size_t emit_save( char* buf, size_t offset) {
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

static size_t emit_restore( char* buf, size_t offset) {
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
static size_t emit_fault(char* buf, size_t offset) {
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

static size_t emit_call_abs(long callee, char* buf, size_t offset) {
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
static size_t emit_call_jump(long callee, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push callee address
  insnsize = emit_push_imm64(callee, p, 0);
  p += insnsize;
  // ret
  *p++ = 0xc3;

  return (p - (buf + offset));
}

static size_t emit_align_stack(int pushed_size, char* buf, size_t offset) {
  char* p = buf + offset;
  // push %rax
  *p++ = (char)0x50;

  // lea (rsp), rax
  *p++ = (char)0x48;
  *p++ = (char)0x8d;
  *p++ = (char)0x04;
  *p++ = (char)0x24;
  /*
  *p++ = (char)0x00;
  *p++ = (char)0x00;
  *p++ = (char)0x00;
  *p++ = (char)0x00;
  */
  //*((int*)p) = (int)pushed_size;
  //p+=4;

  // andq 0xfffffffffffffff0, rsp
  *p++ = (char)0x48;
  *p++ = (char)0x83;
  *p++ = (char)0xe4;
  *p++ = (char)0xf0;

  // mov rax, -8[rsp]
  *p++ = (char)0x48;
  *p++ = (char)0x89;
  *p++ = (char)0x44;
  *p++ = (char)0x24;
  *p++ = (char)0xf8;

  return (p - (buf + offset));
}

static size_t emit_unalign_stack(char* buf, size_t offset) {
  char* p = buf + offset;
  // move -8[%rsp], %rsp
  *p++ = (char)0x48;
  *p++ = (char)0x8b;
  *p++ = (char)0x64;
  *p++ = (char)0x24;
  *p++ = (char)0xf8;

  // pop %rax
  *p++ = (char)0x58;

  return (p - (buf + offset));
}

static size_t emit_jump_abs(long trg, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  // push jump target
  insnsize = emit_push_imm64(trg, p, 0);
  p += insnsize;
  // ret
  *p = 0xc3;
  //*p = 0xcb;

  p ++;

  return (p - (buf + offset));
}

SpSnippet::SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
                     Dyninst::PatchAPI::Point* pt,
                     SpContext* c,
                     PayloadFunc p)
  : func_(f), point_(pt), context_(c), payload_(p), blob_size_(0), old_context_(NULL) {
  // FIXME: use AddrSpace::malloc later
  assert(context_ && "SpContext is NULL");
  blob_ = (char*)malloc(1024);
  setcontext_func_ = context_->setcontext_func();
  getcontext_func_ = context_->getcontext_func();
}

SpSnippet::~SpSnippet() {
  free(blob_);
  free(old_context_);
}
/* Psuedo Assembly for Blob:
   1. Save context
   2. Call payload function
   3. Restore context
   4. Call original function
 */
char* SpSnippet::blob(Dyninst::Address ret_addr) {
  assert(payload_);
  assert(context_);
  assert(func_);
  sp_debug("BLOB - patch area at %lx for calling %s, will return to %lx",
          blob_, func_->name().c_str(), ret_addr);
  if (blob_size_ > 0) {
    sp_debug("BLOB - Blob is constructed for calling %s(), just grab it!",
            func_->name().c_str());
    return blob_;
  }
  /*
  long* stack = (long*)old_context_->uc_mcontext.gregs[REG_RSP];
  for (int i = 0; i < 10; i++) {
    sp_debug("STACK @ %lx: %lx", &stack[i], stack[i]);
  }
*/
  // Build blob

  // movq %rdi, old_context_
  size_t offset = 0;
  size_t insnsize = 0;
  //#if 0
  // save context
  insnsize = emit_save(blob_, offset);
  offset += insnsize;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point_, blob_, offset);
  offset += insnsize;

  // movq SP_CONTEXT, %rsi
  insnsize = emit_mov_imm64_rsi((long)context_, blob_, offset);
  offset += insnsize;

  // call payload
  insnsize = emit_call_abs((long)payload_, blob_, offset);
  offset += insnsize;

  // restore context
  insnsize = emit_restore(blob_, offset);
  offset += insnsize;
  /*
  if (func_->name().compare("strtod_l") == 0) {
    sp_debug("STRTOD_L - will crash");
    // setenv SP_COREDUMP first, then cause segfault, then check memory image
    insnsize = emit_fault(blob_, offset);
    offset += insnsize;
  }
*/
  //#endif

  // stack alignment
  //  insnsize = emit_align_stack(0, blob_, offset);
  //  offset += insnsize;

  // call ORIG_FUNCTION and jmp back to original return address
  if (ret_addr == 0) {
    insnsize = emit_call_jump((long)func_->addr(), blob_, offset);
    offset += insnsize;
  } else {
    insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
    offset += insnsize;
    // jmp ORIG_INSN_ADDR
    insnsize = emit_jump_abs(ret_addr, blob_, offset);
    offset += insnsize;
  }

  // stack unalignment
  //  insnsize = emit_unalign_stack(blob_, offset);
  //  offset += insnsize;

  blob_size_ = offset;

  sp_debug("DUMP INSN (%d bytes)- {", offset);
  sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  sp_debug("DUMP INSN - }");

  return blob_;
}

Dyninst::Address get_pre_signal_pc(void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  return ctx->uc_mcontext.gregs[REG_RIP];
}

Dyninst::Address set_pc(Dyninst::Address pc, void* context) {
  ucontext_t* ctx = (ucontext_t*)context;
  ctx->uc_mcontext.gregs[REG_RIP] = pc;
}

}
