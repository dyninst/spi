#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"

using sp::SpContext;
using sp::SpParser;
using sp::SpEvent;

extern sp::SpContextPtr g_context;

Dyninst::Address get_cur_func_ip(void* context) {

  ucontext_t* c = (ucontext_t*)context;
  mcontext_t m = c->uc_mcontext;
  unsigned long rip = m.gregs[REG_RIP];
  sp_debug("rip: %lx", rip);
  return rip;
}

