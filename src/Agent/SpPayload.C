#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"

using sp::SpContextPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;

bool default_payload(Dyninst::PatchAPI::PatchFunction* cur_func,
                     SpContextPtr context) {
  sp_debug("DEFAULT PAYLOAD - Instrumenting function %s", cur_func->name().c_str());
  const PatchFunction::blockset &calls = cur_func->calls();
  sp_debug("DEFAULT PAYLOAD - %d callees", calls.size());
  const PatchFunction::blockset &blks = cur_func->blocks();
  Dyninst::PatchAPI::PatchBlock* b = *blks.begin();
  sp_debug("DEFAULT PAYLOAD - %d blocks, addr %lx, size %lx", blks.size(), b->start(), b->size());
  sp_debug("%s", context->parser()->dump_insn((void*)b->start(), b->size()).c_str());
  sp_debug("callee addr: %lx", context->parser()->get_func_addr(cur_func->name()));
  /*
  std::vector<Point*> pts;
  PatchMgrPtr mgr = context->mgr();
  Scope scope(cur_func);
  if (!mgr->findPoints(scope, Point::PreCall, back_inserter(pts))) {
    sp_debug("NO POINT - cannot find any point for function %s", cur_func->name().c_str());
    return false;
  }
  sp_debug("POINT - %d points found in function %s", pts.size(), cur_func->name().c_str());
  */
  for (PatchFunction::blockset::iterator ci = calls.begin(); ci != calls.end(); ci++) {
    PatchFunction* callee = (*ci)->getCallee();
    context->init_propeller()->go(callee, context, context->init_payload());
  }
}

void simple_payload() {
  sp_debug("SIMPLE PAYLOAD - I'm in!");
}
