#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"

using sp::SpContextPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;

bool default_payload(Dyninst::PatchAPI::PatchFunction* cur_func_plt,
                     SpContextPtr context) {
  sp_debug("DEFAULT PAYLOAD - Instrumenting function %s", cur_func_plt->name().c_str());
  printf("%s\n", cur_func_plt->name().c_str());
  PatchFunction* cur_func = context->parser()->findFunction(cur_func_plt->name());
  if (!cur_func) {
    sp_debug("DEFAULT PAYLOAD - Leaf function");
    return false;
  }
  context->init_propeller()->go(cur_func, context, context->init_payload());
  return true;
}

void simple_payload() {
  sp_debug("SIMPLE PAYLOAD - I'm in!");
}
