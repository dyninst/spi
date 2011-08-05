#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

namespace sp {


char* SpSnippet::blob() {
  sp_debug("BLOB - Invoke payload %lx with parameters 1) context %lx and 2) func %s",
           payload_, context_.get(), func_->name().c_str());
  return NULL;
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
