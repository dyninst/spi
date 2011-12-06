#include "SpPropeller.h"
#include "SpContext.h"
#include "SpSnippet.h"
#include "SpUtils.h"

using sp::SpContext;
using sp::SpPropeller;

using ph::Point;
using ph::Scope;
using ph::Patcher;
using ph::Snippet;
using ph::PatchMgr;
using ph::PatchBlock;
using ph::PatchMgrPtr;
using ph::PatchFunction;
using ph::PushBackCommand;

namespace sp {

SpPropeller::SpPropeller() {
}

SpPropeller::ptr
SpPropeller::create() {
  return ptr(new SpPropeller);
}

/* Instrument all "interesting points" inside `func`.
   "Interesting points" are provided by SpPropeller::next_points.
   By default, next_points provides all call instructions of `func`.
   However, users can inherit SpPropeller and implement their own next_points().
*/
bool
SpPropeller::go(PatchFunction* func, SpContext* context, PayloadFunc before,
                PayloadFunc after, Point* pt) {
#ifndef SP_RELEASE
  sp_debug("START PROPELLING - propel to callees of function %s", func->name().c_str());
#endif
  /* 1. Find points according to type */
  Points pts;
  PatchMgrPtr mgr = context->mgr();
  PatchFunction* cur_func = NULL;
  if (pt) {
    cur_func = func;
  } else {
    cur_func = context->parser()->findFunction(func->name());
  }
  next_points(cur_func, mgr, pts);

  /* 2. Start instrumentation */
  ph::Patcher patcher(mgr);
  for (int i = 0; i < pts.size(); i++) {
    Point* pt = pts[i];

    /* It's possible that callee will be NULL, which is an indirect call.
       In this case, we'll parse it later during runtime.
    */ 
    PatchFunction* callee = context->parser()->callee(pt);
#ifndef SP_RELEASE
    if (callee) {
      sp_debug("POINT - instrumenting direct call at %lx to function %s",
	       pt->block()->last(), callee->name().c_str());
    } else {
      sp_debug("POINT - instrumenting indirect call at %lx", pt->block()->last());
    }
#endif
    SpSnippet::ptr sp_snip = SpSnippet::create(callee, pt, context, before, after);
    Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::create(sp_snip);
    patcher.add(PushBackCommand::create(pt, snip));
  }
  patcher.commit();
#ifndef SP_RELEASE
  sp_debug("FINISH PROPELLING - %d callees of function %s are instrumented", pts.size(), func->name().c_str());
#endif
  return true;
}

/* Find all PreCall points */
void
SpPropeller::next_points(PatchFunction* cur_func, PatchMgrPtr mgr, Points& pts) {
  Scope scope(cur_func);
  mgr->findPoints(scope, Point::PreCall, back_inserter(pts));
}

}
