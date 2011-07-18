#include "Agent.h"
#include "Common.h"

using sp::Agent;
using sp::Parser;
using sp::Context;
using sp::ContextPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchObject;

/* Constructor for Agent */
Agent::ptr Agent::create() {
  return ptr(new Agent());
}

Agent::Agent() {
  sp_debug("%s", __FUNCTION__);
  init_event_ = Event::ptr();
  fini_event_ = Event::ptr();
  parser_ = Parser::ptr();
}

/* Configuration */
void Agent::setParser(Parser::ptr parser) {
  sp_debug("%s", __FUNCTION__);
  parser_ = parser;
}

void Agent::setInitEvent(Event::ptr e) {
  sp_debug("%s", __FUNCTION__);
  init_event_ = e;
}

void Agent::setFiniEvent(Event::ptr e) {
  sp_debug("%s", __FUNCTION__);
  init_event_ = e;
}

void Agent::setPayload(Payload::ptr p) {
  sp_debug("%s", __FUNCTION__);
  init_payload_ = p;
}

void Agent::setPropeller(Propeller::ptr p) {
  sp_debug("%s", __FUNCTION__);
  propeller_ = p;
}

/* Go! */
void Agent::go() {
  sp_debug("%s", __FUNCTION__);

  // 0. Sanity check. If not user configuration, use default ones
  if (!init_event_) init_event_ = NowEvent::create();
  if (!fini_event_) fini_event_ = Event::create();
  if (!parser_) parser_ = Parser::create();
  if (!init_payload_) init_payload_ = Payload::create();
  if (!propeller_) propeller_ = Propeller::create();

  // 1. Parsing and initialize PatchAPI stuffs
  Parser::PatchObjects& cos = parser_->parse();
  sp_debug("%d PatchObjects created", cos.size());

  PatchObject* exe_obj = parser_->exe_obj();
  assert(exe_obj);

  sp_debug("PatchObject for exe with load address 0x%lx", exe_obj->codeBase());
  AddrSpacePtr as = AddrSpace::create(exe_obj);
  mgr_ = PatchMgr::create(as);

  for (Parser::PatchObjects::iterator i = cos.begin(); i != cos.end(); i++) {
    if (*i != exe_obj) {
      as->loadObject(*i);
      sp_debug("PatchObject for .so with load address 0x%lx", (*i)->codeBase());
    }
  }

  // 2. Prepare context
  // TODO (wenbin): what's in the context?
  // - init_payload
  // - PatchMgr
  ContextPtr context = Context::create(propeller_,
                                       init_payload_,
                                       mgr_);

  // 3. Register Events
  init_event_->register_event(context);
  fini_event_->register_event(context);
}
