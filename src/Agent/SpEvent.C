#include "SpEvent.h"
#include "SpUtils.h"
#include "SpParser.h"
#include "SpContext.h"

namespace sp {
  extern SpContext* g_context;
	extern SpParser::ptr g_parser;

  // Default Event -- dumb event, does nothing
  SpEvent::SpEvent() {
  }

  void
  SpEvent::register_event() {
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
  AsyncEvent::register_event() {
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
  SyncEvent::register_event() {
    if (!g_parser->injected()) {
#ifndef SP_RELEASE
      sp_debug("PRELOAD - preload agent.so, and instrument main()");
#endif
			ph::PatchFunction* f = g_parser->findFunction("main");
      g_context->init_propeller()->go(f,
																			g_context->init_entry(),
																			g_context->init_exit());
    } // LD_PRELOAD mode

    else {

      // Instrument all functions in the call stack.
      FuncSet call_stack;
      g_context->get_callstack(&call_stack);
      sp_debug("CALLSTACK - %lu calls in the call stack",
							 (unsigned long)call_stack.size());
      for (FuncSet::iterator i = call_stack.begin(); 
					 i != call_stack.end(); i++) {
				ph::PatchFunction* f = *i;
        g_context->init_propeller()->go(f,
                                        g_context->init_entry(),
                                        g_context->init_exit());
        if (f->name().compare("main") == 0) {
          break;
        }
      } // Call stack
    } // Injection mode
  }
}
