#include "SpPayload.h"
#include "PatchCFG.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::Scope;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchObject;

void default_payload(Point* pt, sp::SpContext* context) {

  sp_debug("DEFAULT PAYLOAD - Instrumenting function %s", pt->getCallee()->name().c_str());
  sp_print("%s", pt->getCallee()->name().c_str());

  sp_debug("CALL BLOCK per iter {");
  sp_debug("%s", context->parser()->dump_insn((void*)pt->block()->start(), pt->block()->size()).c_str());
  sp_debug("}");

/*
  PatchObject* obj = pt->block()->obj();
  vector<PatchFunction*> funcs;
  obj->funcs(back_inserter(funcs));
  for (int i = 0; i < funcs.size(); i++) {
    const PatchFunction::Blockset& blocks = funcs[i]->blocks();
    for (PatchFunction::Blockset::const_iterator bi = blocks.begin(); bi != blocks.end(); bi++) {
      sp_debug("Every Block code {");
      sp_debug("%s", context->parser()->dump_insn((void*)(*bi)->start(), (*bi)->size()).c_str());
      sp_debug("}");
    }
  }

  vector<PatchBlock*> blks;
  obj->blocks(back_inserter(blks));
  sp_debug("%d blocks", blks.size());
  for (int i = 0; i < blks.size(); i++) {
  sp_debug("Every Block code {");
  sp_debug("%s", context->parser()->dump_insn((void*)blks[i]->start(), blks[i]->size()).c_str());
  sp_debug("}");
  }

  PatchBlock* blk = pt->block();
  const PatchBlock::edgelist& edges = blk->getTargets();

  for (PatchBlock::edgelist::const_iterator ei = edges.begin(); ei != edges.end(); ei++) {
    PatchEdge* e = *ei;
    PatchBlock* b = e->target();
    sp_debug("Target BLOCK per iter {");
    sp_debug("%s", context->parser()->dump_insn((void*)b->start(), b->size()).c_str());
    sp_debug("}");
  }

  FILE* fp=fopen("/tmp/vlog", "a");
  fprintf(fp, "%s\n", pt->getCallee()->name().c_str());
  fclose(fp);
  */

  sp::Points pts;
  sp::CalleePoints(pt->getCallee(), context, pts);
  sp_debug("FIND POINTS - %d points found in function %s", pts.size(), pt->getCallee()->name().c_str());
  if (pts.size() > 0) {
    sp::SpPropeller::ptr p = sp::SpPropeller::create();
    p->go(pts, context, context->init_payload());
  } else {
    sp_debug("NO POINTS - to propell");
  }
}

bool simple_payload(Point* pt, sp::SpContext* context) {
  sp_debug("SIMPLE PAYLOAD - I'm in %s", pt->getCallee()->name().c_str());
  sp_print("%s", pt->getCallee()->name().c_str());
  return true;
}
