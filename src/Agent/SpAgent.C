#include "SpAgent.h"
#include "SpContext.h"

using sp::SpAgent;
using sp::SpParser;
using sp::SpContext;

using ph::PatchMgr;
using ph::AddrSpace;
using ph::PatchObject;

using pe::CodeObject;

/* Constructor for SpAgent */
SpAgent::ptr
SpAgent::create() {

  /* Enable core dump. */
  if (getenv("SP_COREDUMP")) {
    struct rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;
    core_limit.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
      sp_perror("ERROR: failed to setup core dump ability\n");
    }
  }
  return ptr(new SpAgent());
}

SpAgent::SpAgent() {

  init_event_ = SpEvent::ptr();
  fini_event_ = SpEvent::ptr();
  parser_ = SpParser::ptr();

  parse_only_ = false;
  directcall_only_ = false;
  allow_ipc_ = false;
}

SpAgent::~SpAgent() {
  /* FIXME: Very weird segfault when uncommenting this. */
  // delete context_;
}

/* Configuration */
void
SpAgent::set_parser(SpParser::ptr parser) {
  parser_ = parser;
}

void
SpAgent::set_init_event(SpEvent::ptr e) {
  init_event_ = e;
}

void
SpAgent::set_fini_event(SpEvent::ptr e) {
  init_event_ = e;
}

void
SpAgent::set_init_before(string p) {
  init_before_ = p;
}

void
SpAgent::set_init_after(string p) {
  init_after_ = p;
}

void
SpAgent::set_init_propeller(SpPropeller::ptr p) {
  init_propeller_ = p;
}

void
SpAgent::set_parse_only(bool b) {
  parse_only_ = b;
}

void
SpAgent::set_directcall_only(bool b) {
  directcall_only_ = b;
}

void
SpAgent::set_ipc(bool b) {
  allow_ipc_ = b;
}

/* Here We Go! Self-propelling magic happens! */
void
SpAgent::go() {

#ifndef SP_RELEASE
  sp_debug("========== Start Self-propelled instrumentation @ Process %d ==========", getpid());
#endif

  /* Sanity check. If not user-provided configuration, use default ones */
  if (!init_event_) {
#ifndef SP_RELEASE
    sp_debug("INIT EVENT - Use default event");
#endif
    init_event_ = SyncEvent::create();
  }
  if (!fini_event_) {
#ifndef SP_RELEASE
    sp_debug("FINI EVENT - Use default event");
#endif
    fini_event_ = SpEvent::create();
  }
  if (init_before_.size() == 0) {
#ifndef SP_RELEASE
    sp_debug("BEFORE_PAYLOAD - Use default payload before calls");
#endif
    init_before_ = "default_before";
  }
  if (init_after_.size() == 0) {
#ifndef SP_RELEASE
    sp_debug("AFTER_PAYLOAD - No payload after calls");
#endif
    init_after_ = "";
  }
  if (!parser_) {
#ifndef SP_RELEASE
    sp_debug("PARSER - Use default parser");
#endif
    parser_ = SpParser::create();
  }
  if (!init_propeller_) {
#ifndef SP_RELEASE
    sp_debug("PROPELLER - Use default propeller");
#endif

    init_propeller_ = SpPropeller::create();
  }
#ifndef SP_RELEASE
  if (directcall_only_) {
    sp_debug("DIRECT CALL ONLY - only instrument direct calls, ignoring indirect calls");
  } else {
    sp_debug("DIRECT/INDIRECT CALL - instrument both direct and indirect calls");
  }
  if (allow_ipc_) {
    sp_debug("MULTI PROCESS - support multiprocess instrumentation");
  } else {
    sp_debug("SINGLE PROCESS - only support single-process instrumentation");
  }
#endif

  /* Prepare context */
  context_ = SpContext::create(init_propeller_,
                               init_before_,
                               init_after_,
                               parser_);
  context_->set_directcall_only(directcall_only_);
  context_->set_allow_ipc(allow_ipc_);

  if (parse_only_) {
#ifndef SP_RELEASE
    sp_debug("PARSE ONLY - exit after parsing, without instrumentation");
#endif

    return;
  }

  /* Register Events */
  init_event_->register_event(context_);
  fini_event_->register_event(context_);
}
