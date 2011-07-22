#include <ucontext.h>

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


/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
  sp_debug("%s", __FUNCTION__);
}

void SpEvent::register_event(SpContextPtr c) {
  sp_debug("Event::%s", __FUNCTION__);
}

/* SyncEvent */
SyncEvent::SyncEvent(std::string func_name) : func_name_(func_name) {
  sp_debug("%s", __FUNCTION__);
}


void SyncEvent::register_event(SpContextPtr c) {
  sp_debug("SyncEvent::%s", __FUNCTION__);
  c->parse();
  if (func_name_.size() == 0) {
    Dyninst::Address ip = c->parser()->findGlobalVar(IJ_PC_VAR);
    c->parser()->findFunction(ip);
  } else {
    // find function
  }
  c->propel(SpPropeller::CALLEE, c->init_payload());
}

/* AsyncEvent */

sp::SpContextPtr g_context = sp::SpContextPtr();
extern Dyninst::Address get_cur_func_ip(void* context);

void event_handler(int signum, siginfo_t* info, void* context) {
  g_context->parse();
  Dyninst::Address ip = get_cur_func_ip(context);
  SpParser::ptr parser = g_context->parser();
  parser->findFunction(ip);
  g_context->propel(SpPropeller::CALLEE, g_context->init_payload());
}

AsyncEvent::AsyncEvent(int signum, int sec)
  : after_secs_(sec), signum_(signum) {
  sp_debug("%s", __FUNCTION__);
}

void AsyncEvent::register_event(SpContextPtr c) {
  sp_debug("TimerEvent::%s", __FUNCTION__);
  g_context = c;

  struct sigaction act;
  act.sa_sigaction = event_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(signum_, &act, NULL);
  if (signum_ == SIGALRM) alarm(after_secs_);
}
