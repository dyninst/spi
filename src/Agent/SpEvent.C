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
  sp::g_context->parse();
  PatchFunction* f = sp::g_context->get_first_inst_func();
  g_context->init_propeller()->go(f, g_context,
                                  g_context->init_before(),
                                  g_context->init_after());
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
void
sync_event_handler(int signum, siginfo_t* info, void* context) {
  g_context->parse();
  PatchFunction* f = g_context->get_first_inst_func();
  g_context->init_propeller()->go(f, g_context,
                                  g_context->init_before(),
                                  g_context->init_after());
}

SyncEvent::SyncEvent(std::string func_name, int sec)
  : AsyncEvent(SIGALRM, sec), func_name_(func_name) {
  handler_ = (void*)sync_event_handler;
}


void
SyncEvent::register_event(SpContext* c) {
  g_context = c;

  if (!g_context->parser()->injected()) {
#ifndef SP_RELEASE
    sp_debug("PRELOAD - preload agent.so, and instrument main()");
#endif
    PatchFunction* f = c->parser()->findFunction("main");
    c->init_propeller()->go(f, c, c->init_before(), c->init_after());
  } else {
    sp_print("We are triggered!");

    /* 
       1. Get the last instruction we stopped
       2. Find the function that contains it
       3. Stackwalk until main
       4. Instrument all functions in the stack 
    */

    /*
    struct sigaction act;
    act.sa_sigaction = (event_handler_t)handler_;
    act.sa_flags = SA_SIGINFO;
    sigaction(signum_, &act, NULL);
    if (signum_ == SIGALRM) alarm(after_secs_);
    */
    /*
    PatchFunction* f = g_context->get_first_inst_func();
    sp_print("FIRST INST -- in function %s", f->name().c_str());
    g_context->init_propeller()->go(f, g_context,
                                  g_context->init_before(),
                                  g_context->init_after());
    */
  }
}

}
