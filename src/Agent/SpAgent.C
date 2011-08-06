#include <unistd.h>
#include <sys/resource.h>
#include "SpCommon.h"
#include "SpAgent.h"
#include "SpContext.h"

using sp::SpAgent;
using sp::SpParser;
using sp::SpContext;
using sp::SpContextPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchObject;

/* Constructor for SpAgent */
SpAgent::ptr SpAgent::create() {
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
  sp_debug("========== Agent ==========");
  init_event_ = SpEvent::ptr();
  fini_event_ = SpEvent::ptr();
  parser_ = SpParser::ptr();
}

SpAgent::~SpAgent() {
}

/* Configuration */
void SpAgent::set_parser(SpParser::ptr parser) {
  parser_ = parser;
}

void SpAgent::set_init_event(SpEvent::ptr e) {
  init_event_ = e;
}

void SpAgent::set_fini_event(SpEvent::ptr e) {
  init_event_ = e;
}

void SpAgent::set_init_payload(string p) {
  init_payload_ = p;
}

void SpAgent::set_propeller(SpPropeller::ptr p) {
  init_propeller_ = p;
}

/* Go! */
void SpAgent::go() {
  // 1. Sanity check. If not user configuration, use default ones
  if (!init_event_) init_event_ = SyncEvent::create();
  if (!fini_event_) fini_event_ = SpEvent::create();
  if (init_payload_.size() == 0) init_payload_ = "default_payload";
  if (!parser_) parser_ = SpParser::create();
  if (!init_propeller_) init_propeller_ = SpPropeller::create();

  // 2. Prepare context
  SpContextPtr context = SpContext::create(init_propeller_,
                                           init_payload_,
                                           parser_);

  // 3. Register Events
  init_event_->register_event(context);
  fini_event_->register_event(context);
}
