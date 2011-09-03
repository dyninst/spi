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

SpSnippet::SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
                     Dyninst::PatchAPI::Point* pt,
                     SpContextPtr c,
                     PayloadFunc p)
  : func_(f), point_(pt), context_(c), payload_(p), blob_size_(0), old_context_(NULL) {
  // FIXME: use AddrSpace::malloc later
  blob_ = (char*)malloc(1024);
}

SpSnippet::~SpSnippet() {
  free(blob_);
  free(old_context_);
}
/* Assembly for Blob:

    movq OLD_CONTEXT, %rdi
    callq getcontext
    movq POINT, %rdi
    movq SP_CONTEXT, %rsi
    callq payload
    movq OLD_CONTEXT, %rdi
    callq setcontext
    callq ORIG_FUNCTION
 */
char* SpSnippet::blob() {
  assert(payload_);
  assert(context_);
  assert(func_);
  sp_debug("BLOB - Invoke payload %lx with parameters 1) context %lx and 2) func %s",
           payload_, context_.get(), func_->name().c_str());

  sp_debug("BLOB - Constructing a blob");

  // Allocate buffer for old_context
  old_context_ = (ucontext_t*)malloc(sizeof(ucontext_t));

  // Get setcontext and getcontext
  PatchFunction* setcontext_func = context_->parser()->findFunction("setcontext", false);
  PatchFunction* getcontext_func = context_->parser()->findFunction("getcontext", false);

  sp_debug("VARIABLES IN BLOB - old_context: %lx; point: %lx; sp_context: %lx",
           old_context_, point_, context_.get());
  sp_debug("FUNCTIONS IN BLOB - setcontext: %lx; getcontext: %lx; payload: %lx; orig_func: %lx",
           setcontext_func->addr(), getcontext_func->addr(), payload_, func_->addr());

  // Build blob
  // movq %rdi, old_context_
  size_t offset = 0;
  size_t insnsize = emit_mov_imm64_rdi((long)old_context_, blob_, offset);
  offset += insnsize;

  // callq getcontext
  insnsize = emit_call_abs((long)getcontext_func->addr(), blob_, offset);
  offset += insnsize;

  // movq POINT, %rdi
  insnsize = emit_mov_imm64_rdi((long)point_, blob_, offset);
  offset += insnsize;

  // movq SP_CONTEXT, %rsi
  insnsize = emit_mov_imm64_rsi((long)context_.get(), blob_, offset);
  offset += insnsize;

  // callq payload
  insnsize = emit_call_abs((long)payload_, blob_, offset);
  offset += insnsize;


  // movq old_context, %rdi
  insnsize = emit_mov_imm64_rdi((long)old_context_, blob_, offset);
  offset += insnsize;

  // callq setcontext
  insnsize = emit_call_abs((long)setcontext_func->addr(), blob_, offset);
  offset += insnsize;

  // movq ORIG_FUNCTION
  insnsize = emit_call_abs((long)func_->addr(), blob_, offset);
  offset += insnsize;

  blob_size_ = offset;
  //sp_debug("DUMP INSN (%d bytes)- {", offset);
  //sp_debug("%s", context_->parser()->dump_insn((void*)blob_, offset).c_str());
  //sp_debug("DUMP INSN - }");

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
