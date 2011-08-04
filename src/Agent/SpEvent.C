#include <ucontext.h>

#include "PatchCFG.h"
#include "SpEvent.h"
#include "SpContext.h"
#include "signal.h"
#include "SpParser.h"

using sp::SpParser;
using sp::SpEvent;
using sp::SyncEvent;
using sp::AsyncEvent;
using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;
using Dyninst::PatchAPI::PatchFunction;

/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
}

void SpEvent::register_event(SpContextPtr c) {
}


/* AsyncEvent */
typedef void (*event_handler_t)(int, siginfo_t*, void*);

sp::SpContextPtr g_context = sp::SpContextPtr();
void async_event_handler(int signum, siginfo_t* info, void* context) {
  PatchFunction* f = g_context->get_first_inst_func();
  g_context->init_propeller()->go(f, g_context, g_context->init_payload());
}

AsyncEvent::AsyncEvent(int signum, int sec)
  : after_secs_(sec), signum_(signum) {
  handler_ = (void*)async_event_handler;
}

void AsyncEvent::register_event(SpContextPtr c) {
  g_context = c;

  // g_context->parse();

  struct sigaction act;
  act.sa_sigaction = (event_handler_t)handler_;
  act.sa_flags = SA_SIGINFO;
  sigaction(signum_, &act, NULL);
  if (signum_ == SIGALRM) alarm(after_secs_);
}

/* SyncEvent */
void sync_event_handler(int signum, siginfo_t* info, void* context) {
  PatchFunction* f = g_context->get_first_inst_func();
  g_context->init_propeller()->go(f, g_context, g_context->init_payload());
}

SyncEvent::SyncEvent(std::string func_name, int sec)
  : AsyncEvent(SIGALRM, sec), func_name_(func_name) {
  handler_ = (void*)sync_event_handler;
}

