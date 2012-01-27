#include "SpEvent.h"
#include "SpUtils.h"
#include "SpParser.h"
#include "SpContext.h"

using sp::SpEvent;
using sp::SpParser;
using sp::SpContext;
using sp::SyncEvent;
using sp::AsyncEvent;
using sp::SpPropeller;

using ph::PatchFunction;

namespace sp {
  extern SpContext* g_context;

  // Default Event -- dumb event, does nothing
  SpEvent::SpEvent() {
  }

  void
  SpEvent::register_event(SpContext* c) {
  }

  // AsyncEvent
  typedef void (*event_handler_t)(int, siginfo_t*, void*);

  void
  async_event_handler(int signum, siginfo_t* info, void* context) {
    assert(0 && "TODO");
  }

  AsyncEvent::AsyncEvent(int signum, int sec)
    : after_secs_(sec), signum_(signum) {
    handler_ = (void*)async_event_handler;
  }

  void
  AsyncEvent::register_event(SpContext* c) {
    struct sigaction act;
    act.sa_sigaction = (event_handler_t)handler_;
    act.sa_flags = SA_SIGINFO;
    sigaction(signum_, &act, NULL);
    if (signum_ == SIGALRM) alarm(after_secs_);
  }

  // SyncEvent
  SyncEvent::SyncEvent(std::string func_name)
    : SpEvent(), func_name_(func_name) {
  }


  void
  SyncEvent::register_event(SpContext* c) {
    if (!g_context->parser()->injected()) {
#ifndef SP_RELEASE
      sp_debug("PRELOAD - preload agent.so, and instrument main()");
#endif
      PatchFunction* f = c->parser()->findFunction("main");
      c->init_propeller()->go(f, c, c->init_entry(), c->init_exit());
    } // LD_PRELOAD mode

    else {

      // Instrument all functions in the call stack.
      FuncSet call_stack;
      g_context->get_callstack(&call_stack);
      sp_debug("CALLSTACK - %lu calls in the call stack", (unsigned long)call_stack.size());
      for (FuncSet::iterator i = call_stack.begin(); 
					 i != call_stack.end(); i++) {
        PatchFunction* f = *i;
        g_context->init_propeller()->go(f, g_context,
                                        g_context->init_entry(),
                                        g_context->init_exit());
        if (f->name().compare("main") == 0) {
          break;
        }
      } // Call stack
    } // Injection mode
  }
}
