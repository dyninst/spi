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
  *p++ = 0x57; // push rdi
  *p++ = 0x56; // push rsi
  *p++ = 0x52; // push rdx
  *p++ = 0x51; // push rcx

  *p++ = 0x41; // r8
  *p++ = 0x50;

  *p++ = 0x41; // r9
  *p++ = 0x51;

  return (p - (buf + offset));
}

static size_t emit_restore( char* buf, size_t offset) {
  char* p = buf + offset;
  *p++ = 0x41; // pop r8
  *p++ = 0x59;

  *p++ = 0x41; // pop r8
  *p++ = 0x58;

  *p++ = 0x59; // pop rcx
  *p++ = 0x5a; // pop rdx
  *p++ = 0x5e; // pop rsi
  *p++ = 0x5f; // pop rdi

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

static size_t emit_illegal(char* buf, size_t offset) {
  char* p = buf + offset;
  *p++ = 0xc7;
  *p++ = 0x04;
  *p++ = 0x25;

  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;

  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x00;

  return (p - (buf + offset));
}
  /*
static size_t emit_orig_call_abs(long callee, char* buf, size_t offset, long orig_rsp) {
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
  *p = 0xc3;
  p ++;
  assert(retaddr == (long)p);

  return (p - (buf + offset));
}
  */

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


static size_t emit_restore_regs(ucontext_t* c, char* buf, size_t offset) {
  char* p = buf + offset;
  size_t insnsize = 0;

  sp_debug("IN BLOB - old_context: %lx", c);

  // REG_RDI
  *p = (char)0x48; p++;
  *p = (char)0xbf; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RDI];
  p += sizeof(long);

  // REG_RSI
  *p = (char)0x48; p++;
  *p = (char)0xbe; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RSI];
  p += sizeof(long);


  // REG_RBP
  *p = (char)0x48; p++;
  *p = (char)0xbd; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RBP];
  p += sizeof(long);

  // REG_RBX
  *p = (char)0x48; p++;
  *p = (char)0xbb; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RBX];
  p += sizeof(long);

  // REG_RDX
  *p = (char)0x48; p++;
  *p = (char)0xba; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RDX];
  p += sizeof(long);

  // REG_RAX
  *p = (char)0x48; p++;
  *p = (char)0xb8; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RAX];
  p += sizeof(long);

  // REG_RCX
  *p = (char)0x48; p++;
  *p = (char)0xb9; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RCX];
  p += sizeof(long);

  // FIXME!!!!

  // REG_RSP
  *p = (char)0x48; p++;
  *p = (char)0xbc; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_RSP];
  p += sizeof(long);

  // REG_R8
  *p = (char)0x49; p++;
  *p = (char)0xb8; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R8];
  p += sizeof(long);

  // REG_R9
  *p = (char)0x49; p++;
  *p = (char)0xb9; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R9];
  p += sizeof(long);

  // REG_R10
  *p = (char)0x49; p++;
  *p = (char)0xba; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R10];
  p += sizeof(long);

  // REG_R11
  *p = (char)0x49; p++;
  *p = (char)0xbb; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R11];
  p += sizeof(long);


  // REG_R12
  *p = (char)0x49; p++;
  *p = (char)0xbc; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R12];
  p += sizeof(long);

  // REG_R13
  *p = (char)0x49; p++;
  *p = (char)0xbd; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R13];
  p += sizeof(long);

  // REG_R14
  *p = (char)0x49; p++;
  *p = (char)0xbe; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R14];
  p += sizeof(long);

  // REG_R15
  *p = (char)0x49; p++;
  *p = (char)0xbf; p++;
  *((long*)p) = (long)c->uc_mcontext.gregs[REG_R15];
  p += sizeof(long);

  // EFLAGS
  p += emit_push_imm64((long)c->uc_mcontext.gregs[REG_EFL], p, 0);
  *p = (char)0x9d; // popfq
  p++;
  //printf("flags: %x\n", c->uc_mcontext.gregs[REG_EFL]);
  /*
  REG_RIP,
# define REG_RIP	REG_RIP
  REG_CSGSFS,		
# define REG_CSGSFS	REG_CSGSFS
  REG_ERR,
# define REG_ERR	REG_ERR
  REG_TRAPNO,
# define REG_TRAPNO	REG_TRAPNO
  REG_OLDMASK,
# define REG_OLDMASK	REG_OLDMASK
  REG_CR2
# define REG_CR2	REG_CR2
*/
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

    movq POINT, %rdi                          movq POINT, %rdi
    movq SP_CONTEXT, %rsi                     movq SP_CONTEXT, %rsi
    callq payload                             push x 4 ret value
                                              push x 4 payload addr
                                              ret
    movq OLD_CONTEXT, %rdi                    movq OLD_CONTEXT, %rdi
    callq setcontext                          push x 4 ret value
                                              push x 4 setcontext value
                                              ret
    callq ORIG_FUNCTION                       push x 4 ret value
                                              push x 4 ORIG_FUNCTION
    jmp ORIG_INSN_ADDR                        push x 4 ORIG_INSN_ADDR
                                              ret
 */
char* SpSnippet::blob(Dyninst::Address ret_addr) {
  assert(payload_);
  assert(context_);
  assert(func_);
  sp_debug("BLOB - patch area at %lx", blob_);
  if (blob_size_ > 0) {
    sp_debug("BLOB - Blob is constructed for calling %s(), just grab it!",
            func_->name().c_str());
    return blob_;
  }

  sp_debug("BLOB - Constructing a blob");

  sp_debug("VARIABLES IN BLOB - old_context: %lx; point: %lx; sp_context: %lx",
           old_context_, point_, context_);
  sp_debug("FUNCTIONS IN BLOB - setcontext: %lx; getcontext: %lx; payload: %lx; orig_func: %lx",
           setcontext_func_, getcontext_func_, payload_, func_->addr());
  sp_debug("RETURN TO - %lx", ret_addr);

  // Build blob

  // movq %rdi, old_context_
  size_t offset = 0;
  size_t insnsize = 0;
  /*
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
*/
  // restore registers
  //insnsize = emit_restore_regs(old_context_, blob_, offset);
  //offset += insnsize;

  // for debug, cause core dump
  //  insnsize = emit_illegal(blob_, offset);
  //  offset += insnsize;

  // call ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
  offset += insnsize;

  // jmp ORIG_INSN_ADDR
  insnsize = emit_jump_abs(ret_addr, blob_, offset);
  offset += insnsize;

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
