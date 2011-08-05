#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"

using sp::SpContextPtr;
using Dyninst::PatchAPI::PatchFunction;

bool default_payload(Dyninst::PatchAPI::PatchFunction* cur_func,
                     SpContextPtr context) {
  sp_debug("DEFAULT PAYLOAD - Instrumenting function %s", cur_func->name().c_str());
  const ParseAPI::Function::edgelist &callEdges = cur_func->function()->callEdges();
  sp_debug("DEFAULT PAYLOAD - %d call edges", callEdges.size());
  const PatchFunction::blockset &calls = cur_func->calls();
  sp_debug("DEFAULT PAYLOAD - %d callees", calls.size());
  for (PatchFunction::blockset::iterator ci = calls.begin(); ci != calls.end(); ci++) {
    PatchFunction* callee = (*ci)->getCallee();
    context->init_propeller()->go(callee, context, context->init_payload());
  }
}

void simple_payload() {
  sp_debug("SIMPLE PAYLOAD - I'm in!");
}
