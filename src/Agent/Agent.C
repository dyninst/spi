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

  // 2. Register Events
}
