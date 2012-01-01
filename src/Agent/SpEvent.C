#include "SpEvent.h"
#include "SpContext.h"
#include "SpParser.h"

using sp::SpEvent;
using sp::SpParser;
using sp::SpContext;
using sp::SyncEvent;
using sp::AsyncEvent;
using sp::SpPropeller;

using ph::PatchFunction;

namespace sp {
/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
}

void
SpEvent::register_event(SpContext* c) {
}


/* AsyncEvent */
typedef void (*event_handler_t)(int, siginfo_t*, void*);

SpContext* g_context = NULL;

void
async_event_handler(int signum, siginfo_t* info, void* context) {
  // TODO
  assert(0 && "TODO");
  /*
  sp::g_context->parse();
  PatchFunction* f = sp::g_context->get_first_inst_func();
  g_context->init_propeller()->go(f, g_context,
                                  g_context->init_before(),
                                  g_context->init_after());
  */
}

AsyncEvent::AsyncEvent(int signum, int sec)
  : after_secs_(sec), signum_(signum) {
  handler_ = (void*)async_event_handler;
}

void
AsyncEvent::register_event(SpContext* c) {
  g_context = c;
  struct sigaction act;
  act.sa_sigaction = (event_handler_t)handler_;
  act.sa_flags = SA_SIGINFO;
  sigaction(signum_, &act, NULL);
  if (signum_ == SIGALRM) alarm(after_secs_);
}

/* SyncEvent */
SyncEvent::SyncEvent(std::string func_name)
  : SpEvent(), func_name_(func_name) {
}


void
SyncEvent::register_event(SpContext* c) {
  g_context = c;

  /* FIXME: ignore bash for now ... should fix it! */
  string exe_name = g_context->parser()->exe_name();
  if (exe_name.compare("bash") == 0) return;

  /* LD_PRELOAD mode */
  if (!g_context->parser()->injected()) {
#ifndef SP_RELEASE
    sp_debug("PRELOAD - preload agent.so, and instrument main()");
#endif
    PatchFunction* f = c->parser()->findFunction("main");
    c->init_propeller()->go(f, c, c->init_before(), c->init_after());
  }
  /* Injection mode */
  else {

    /* Instrument all functions in the call stack. */
    SpContext::CallStack call_stack;
    g_context->get_callstack(&call_stack);
    sp_debug("CALLSTACK - %lu calls in the call stack", call_stack.size());
    for (unsigned i = 0; i < call_stack.size(); i++) {
      PatchFunction* f = call_stack[i];
      g_context->init_propeller()->go(f, g_context,
				      g_context->init_before(),
				      g_context->init_after());
      if (f->name().compare("main") == 0) {
	break;
      }
    } // Call stack
  } // Injection
}

}
