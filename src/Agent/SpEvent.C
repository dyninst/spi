#include <ucontext.h>

#include "PatchCFG.h"
#include "SpEvent.h"
#include "SpContext.h"
#include "signal.h"
#include "SpParser.h"
#include "SpNextPoints.h"

using sp::SpParser;
using sp::SpEvent;
using sp::SyncEvent;
using sp::AsyncEvent;
using sp::SpContext;
using sp::SpPropeller;
using Dyninst::PatchAPI::PatchFunction;

/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
}

void SpEvent::register_event(SpContext* c) {
}


/* AsyncEvent */
typedef void (*event_handler_t)(int, siginfo_t*, void*);

sp::SpContext* g_context = NULL;
void async_event_handler(int signum, siginfo_t* info, void* context) {
  g_context->parse();
  PatchFunction* f = g_context->get_first_inst_func();
  sp::Points pts;
  sp::CalleePoints(f, g_context, pts);
  g_context->init_propeller()->go(pts, g_context,
                                  g_context->init_head(),
                                  g_context->init_tail());
}

AsyncEvent::AsyncEvent(int signum, int sec)
  : after_secs_(sec), signum_(signum) {
  handler_ = (void*)async_event_handler;
}

void AsyncEvent::register_event(SpContext* c) {
  g_context = c;
  struct sigaction act;
  act.sa_sigaction = (event_handler_t)handler_;
  act.sa_flags = SA_SIGINFO;
  sigaction(signum_, &act, NULL);
  if (signum_ == SIGALRM) alarm(after_secs_);
}

/* SyncEvent */
void sync_event_handler(int signum, siginfo_t* info, void* context) {
  g_context->parse();
  PatchFunction* f = g_context->get_first_inst_func();
  sp::Points pts;
  sp::CalleePoints(f, g_context, pts);
  g_context->init_propeller()->go(pts, g_context,
                                  g_context->init_head(),
                                  g_context->init_tail());
}

SyncEvent::SyncEvent(std::string func_name, int sec)
  : AsyncEvent(SIGALRM, sec), func_name_(func_name) {
  handler_ = (void*)sync_event_handler;
}


void SyncEvent::register_event(SpContext* c) {
  g_context = c;

  if (!g_context->parser()->injected()) {
    c->parse();
    PatchFunction* f = c->parser()->findFunction("main");
    sp::Points pts;
    sp::CalleePoints(f, c, pts);
    c->init_propeller()->go(pts, c, c->init_head(), c->init_tail());
  } else {
    struct sigaction act;
    act.sa_sigaction = (event_handler_t)handler_;
    act.sa_flags = SA_SIGINFO;
    sigaction(signum_, &act, NULL);
    if (signum_ == SIGALRM) alarm(after_secs_);
  }
}

