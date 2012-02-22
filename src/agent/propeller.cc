#include "agent/point.h"
#include "agent/snippet.h"
#include "agent/propeller.h"

#include "agent/context.h"
#include "common/utils.h"

#include "patchAPI/h/Command.h"
#include "patchAPI/h/PatchMgr.h"

namespace sp {

	extern SpContext* g_context;
	extern SpParser::ptr g_parser;

  SpPropeller::SpPropeller() {
  }

  SpPropeller::ptr
  SpPropeller::create() {
    return ptr(new SpPropeller);
  }

  // Instrument all "interesting points" inside `func`.
  // "Interesting points" are provided by SpPropeller::next_points.
  // By default, next_points provides all call instructions of `func`.
  // However, users can inherit SpPropeller and implement their own
  // next_points().
  bool
  SpPropeller::go(ph::PatchFunction* func,
                  PayloadFunc entry,
									PayloadFunc exit,
									ph::Point* pt) {
		assert(func);

#ifndef SP_RELEASE
    sp_debug("START PROPELLING - propel to callees of function %s",
             func->name().c_str());
#endif

    // 1. Find points according to type
    Points pts;
		ph::PatchMgrPtr mgr = g_parser->mgr();
		assert(mgr);
		ph::PatchFunction* cur_func = NULL;
    if (pt) {
      cur_func = func;
    } else {
      cur_func = g_parser->findFunction(func->name());
    }

    next_points(cur_func, mgr, pts);

    // 2. Start instrumentation
    ph::Patcher patcher(mgr);
    for (unsigned i = 0; i < pts.size(); i++) {
      SpPoint* pt = static_cast<SpPoint*>(pts[i]);
			assert(pt);
			SpBlock* blk = pt->get_block();
			assert(blk);

			if (blk->isShared()) {
				if (blk->instrumented()) {
					continue;
				}
			}

      // It's possible that callee will be NULL, which is an indirect call.
      // In this case, we'll parse it later during runtime.

			SpFunction* callee = g_parser->callee(pt);

      if (callee) {
        sp_debug("POINT - instrumenting direct call at %lx to "
                 "function %s (%lx) for point %lx",
                 blk->last(), callee->name().c_str(),
                 (dt::Address)callee, (dt::Address)pt);
      } else {
        sp_debug("POINT - instrumenting indirect call at %lx for point %lx",
                 blk->last(), (dt::Address)pt);
      }

      SpSnippet::ptr sp_snip = SpSnippet::create(callee,
                                                 pt,
																								 entry,
																								 exit);
			ph::Snippet<SpSnippet::ptr>::Ptr snip =
        ph::Snippet<SpSnippet::ptr>::create(sp_snip);
			assert(sp_snip && snip);
			pt->set_snip(sp_snip);
      patcher.add(ph::PushBackCommand::create(pt, snip));
    }
    bool ret = patcher.commit();

		if (ret) {
			sp_debug("FINISH PROPELLING - callees of function %s are"
							 " instrumented", func->name().c_str());
		} else {
			sp_debug("FINISH PROPELLING - instrumentation failed for"
							 " callees of %s", func->name().c_str());
		}
    return ret;
  }

  // Find all PreCall points
  void
  SpPropeller::next_points(ph::PatchFunction* cur_func,
													 ph::PatchMgrPtr mgr,
                           Points& pts) {
		ph::Scope scope(cur_func);
    mgr->findPoints(scope, ph::Point::PreCall, back_inserter(pts));
  }

}
