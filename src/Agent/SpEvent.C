#include <ucontext.h>

#include "SpEvent.h"
#include "SpContext.h"
#include "signal.h"
#include "SpParser.h"

using sp::SpParser;
using sp::SpEvent;
using sp::NowEvent;
using sp::TimerEvent;
using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;

sp::SpContextPtr g_context = sp::SpContextPtr();
extern Dyninst::Address get_cur_func_ip(void* context);

void event_handler(int signum, siginfo_t* info, void* context) {
  Dyninst::Address ip = get_cur_func_ip(context);

  g_context->parse();

  SpParser::ptr parser = g_context->parser();
  parser->findFunction(ip);

  g_context->propel(SpPropeller::CALLEE, g_context->init_payload());
}

/* Default Event -- dumb event, does nothing */
SpEvent::SpEvent() {
  sp_debug("%s", __FUNCTION__);
}

void SpEvent::register_event(SpContextPtr c) {
  sp_debug("Event::%s", __FUNCTION__);
}

/* Now Event */
NowEvent::NowEvent() {
  sp_debug("%s", __FUNCTION__);
}


void NowEvent::register_event(SpContextPtr c) {
  sp_debug("NowEvent::%s", __FUNCTION__);
  c->parse();
  Dyninst::Address ip = c->parser()->findGlobalVar(IJ_PC_VAR);
  c->parser()->findFunction(ip);
  c->propel(SpPropeller::CALLEE, c->init_payload());
}

/* Timer Event */

TimerEvent::TimerEvent(int sec) : after_secs_(sec){
  sp_debug("%s", __FUNCTION__);
}

void TimerEvent::register_event(SpContextPtr c) {
  sp_debug("TimerEvent::%s", __FUNCTION__);
  g_context = c;

  struct sigaction act;
  act.sa_sigaction = event_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGALRM, &act, NULL);
  alarm(after_secs_);
}
