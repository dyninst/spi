#include <signal.h>
#include <ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

using Dyninst::PatchAPI::PatchFunction;

namespace sp {

/* Assembly for Blob:

    movq %rdi, OLD_CONTEXT
    callq getcontext
    movq %rdi, POINT
    movq %rsi, SP_CONTEXT
    callq payload
    movq %rdi, OLD_CONTEXT
    callq setcontext
    callq ORIG_FUNCTION
 */
char* SpSnippet::blob() {
  sp_debug("BLOB - Invoke payload %lx with parameters 1) context %lx and 2) func %s",
           payload_, context_.get(), func_->name().c_str());

  if (blob_.size() == 0) {
    sp_debug("BLOB - Constructing a blob");

    // Allocate buffer for old_context
    old_context_ = (ucontext_t*)malloc(sizeof(ucontext_t));

    // Get setcontext and getcontext
    PatchFunction* setcontext_func = context_->parser()->findFunction("setcontext", false);
    PatchFunction* getcontext_func = context_->parser()->findFunction("getcontext", false);

    sp_debug("VARIABLES IN BLOB - old_context: %lx; point: %lx; sp_context: %lx",
             old_context_, point_, 0);
    sp_debug("FUNCTIONS IN BLOB - setcontext: %lx; getcontext: %lx; payload: %lx; orig_func: %lx",
             setcontext_func->addr(), getcontext_func->addr(), payload_, func_->addr());
  }
  return (char*)blob_.c_str();
}

SpSnippet::~SpSnippet() {
  free(old_context_);
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
