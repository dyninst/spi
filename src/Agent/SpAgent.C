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
  return ptr(new SpAgent());
}

SpAgent::SpAgent() {
  sp_debug("%s", __FUNCTION__);
  init_event_ = SpEvent::ptr();
  fini_event_ = SpEvent::ptr();
  parser_ = SpParser::ptr();
}

/* Configuration */
void SpAgent::set_parser(SpParser::ptr parser) {
  sp_debug("%s", __FUNCTION__);
  parser_ = parser;
}

void SpAgent::set_init_event(SpEvent::ptr e) {
  sp_debug("%s", __FUNCTION__);
  init_event_ = e;
}

void SpAgent::set_fini_event(SpEvent::ptr e) {
  sp_debug("%s", __FUNCTION__);
  init_event_ = e;
}

void SpAgent::set_payload(SpPayload::ptr p) {
  sp_debug("%s", __FUNCTION__);
  init_payload_ = p;
}

void SpAgent::set_propeller(SpPropeller::ptr p) {
  sp_debug("%s", __FUNCTION__);
  propeller_ = p;
}

/* Go! */
void SpAgent::go() {
  sp_debug("%s", __FUNCTION__);

  // 0. Sanity check. If not user configuration, use default ones
  if (!init_event_) init_event_ = NowEvent::create();
  if (!fini_event_) fini_event_ = SpEvent::create();
  if (!parser_) parser_ = SpParser::create();
  if (!init_payload_) init_payload_ = SpPayload::create();
  if (!propeller_) propeller_ = SpPropeller::create();

  // 1. Parsing and initialize PatchAPI stuffs
  /*
  SpParser::PatchObjects& cos = parser_->parse();
  sp_debug("%d PatchObjects created", cos.size());

  PatchObject* exe_obj = parser_->exe_obj();
  assert(exe_obj);

  sp_debug("PatchObject for exe with load address 0x%lx", exe_obj->codeBase());
  AddrSpacePtr as = AddrSpace::create(exe_obj);
  mgr_ = PatchMgr::create(as);

  for (SpParser::PatchObjects::iterator i = cos.begin(); i != cos.end(); i++) {
    if (*i != exe_obj) {
      as->loadObject(*i);
      sp_debug("PatchObject for .so with load address 0x%lx", (*i)->codeBase());
    }
  }
  */
  // 2. Prepare context
  SpContextPtr context = SpContext::create(propeller_,
                                           init_payload_,
                                           parser_);

  // 3. Register Events
  init_event_->register_event(context);
  fini_event_->register_event(context);
}
