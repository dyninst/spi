#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

using Dyninst::PatchAPI::PatchFunction;

namespace sp {

// a bunch of code generation functions
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
  *p = 0xc3;
  p ++;
  assert(retaddr == (long)p);

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

    movq OLD_CONTEXT, %rdi                    movq OLD_CONTEXT, %rdi
    callq getcontext                          push x 4 ret value
                                              push x 4 getcontext addr
                                              ret
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

  // Allocate buffer for old_context
  old_context_ = (ucontext_t*)malloc(sizeof(ucontext_t));

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
  insnsize = emit_mov_imm64_rdi((long)old_context_, blob_, offset);
  offset += insnsize;

  // callq getcontext
  insnsize = emit_call_abs((long)getcontext_func_, blob_, offset);
  offset += insnsize;
  */
  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point_, blob_, offset);
  offset += insnsize;

  // movq SP_CONTEXT, %rsi
  insnsize = emit_mov_imm64_rsi((long)context_, blob_, offset);
  offset += insnsize;

  // callq payload
  insnsize = emit_call_abs((long)payload_, blob_, offset);
  offset += insnsize;

  /*
  // movq old_context, %rdi
  insnsize = emit_mov_imm64_rdi((long)old_context_, blob_, offset);
  offset += insnsize;

  // callq setcontext
  insnsize = emit_call_abs((long)setcontext_func_, blob_, offset);
  offset += insnsize;
  // movq ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
  offset += insnsize;
  */
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
