#include "SpPropeller.h"
#include "SpContext.h"
#include "PatchMgr.h"
#include "SpSnippet.h"
#include "Instruction.h"

using sp::SpPropeller;
using sp::SpContext;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::PatchBlock;

SpPropeller::SpPropeller() {
}

SpPropeller::ptr SpPropeller::create() {
  return ptr(new SpPropeller);
}

bool SpPropeller::go(Points&  pts,
                     SpContext* context,
                     PayloadFunc payload) {
  if (pts.size() == 0) return false;

  // 1. Find points according to type
  PatchMgrPtr mgr = context->mgr();
  sp_debug("PROPELLING - To %d points", pts.size());

  // 2. Start instrumentation
  Dyninst::PatchAPI::Patcher patcher(mgr);
  for (int i = 0; i < pts.size(); i++) {
    Point* pt = pts[i];
    PatchFunction* callee = pt->getCallee();


    SpSnippet::ptr sp_snip = SpSnippet::create(callee, pt, context, payload);
    Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::create(sp_snip);
    patcher.add(PushBackCommand::create(pt, snip));
  }
  patcher.commit();
  return true;
}

