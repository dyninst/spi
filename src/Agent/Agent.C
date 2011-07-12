#include "Agent.h"
#include "Common.h"

using sp::Agent;
using sp::Parser;
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
}

/* Configuration */
void Agent::setParser(Parser::ptr parser) {
  sp_debug("%s", __FUNCTION__);
  parser_ = parser;
}

/* Go! */
void Agent::go() {
  sp_debug("%s", __FUNCTION__);

  // 1. Parsing and initialize PatchAPI stuffs
  Parser::CodeObjects& cos = parser_->parse();
  sp_debug("%d CodeObjects created", cos.size());

  CodeObject* exe_co = parser_->exe_co();
  assert(exe_co);

  sp_debug("create PatchObject for exe with load address 0x%x", exe_co->cs()->loadAddress());
  PatchObject* exe_obj = PatchObject::create(exe_co, exe_co->cs()->loadAddress());
  AddrSpacePtr as = AddrSpace::create(exe_obj);
  mgr_ = PatchMgr::create(as);

  for (Parser::CodeObjects::iterator i = cos.begin(); i != cos.end(); i++) {
    CodeObject* co = *i;
    assert(co);
    sp_debug("create PatchObject for a shared library with load address 0x%x", co->cs()->loadAddress());
    PatchObject* obj = PatchObject::create(co, co->cs()->loadAddress());
    as->loadObject(obj);
  }

  // 2. Register Events
}
