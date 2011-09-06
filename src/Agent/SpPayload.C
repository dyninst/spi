#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using sp::SpContextPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;

bool default_payload(Point* pt, SpContextPtr context) {
  sp_debug("DEFAULT PAYLOAD - Instrumenting function %s", pt->getCallee()->name().c_str());
  sp_print("%s", pt->getCallee()->name().c_str());

  FILE* fp=fopen("/tmp/vlog", "a");
  fprintf(fp, "%s\n", pt->getCallee()->name().c_str());
  fclose(fp);
  sp::Points pts;

  sp::CalleePoints(pt->getCallee(), context, pts);
  sp_debug("FIND POINTS - %d points found in function %s", pts.size(), pt->getCallee()->name().c_str());

  sp::SpPropeller::ptr p = sp::SpPropeller::create();
  p->go(pts, context, context->init_payload());

  return true;
}

bool simple_payload(Point* pt, SpContextPtr context) {
  sp_debug("SIMPLE PAYLOAD - I'm in %s", pt->getCallee()->name().c_str());
  return true;
}
