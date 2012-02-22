#include "agent/cfg.h"
#include "agent/maker.h"
#include "agent/point.h"

namespace sp {

	// ------------------------------------------------------------------- 
	// PointMaker
	// -------------------------------------------------------------------
	ph::Point*
	SpPointMaker::mkFuncPoint(ph::Point::Type    t,
														ph::PatchMgrPtr    m,
														ph::PatchFunction* f) {
		return new SpPoint(t, m, f);
	}

	ph::Point*
	SpPointMaker::mkFuncSitePoint(ph::Point::Type    t,
																ph::PatchMgrPtr    m,
																ph::PatchFunction* f,
																ph::PatchBlock*    b) {
		return new SpPoint(t, m, f, b);
	}

	ph::Point*
	SpPointMaker::mkBlockPoint(ph::Point::Type    t,
														 ph::PatchMgrPtr    m,
														 ph::PatchBlock*    b,
														 ph::PatchFunction* f) {
		return new SpPoint(t, m, b, f);
	}

	ph::Point*
	SpPointMaker::mkInsnPoint(ph::Point::Type      t,
														ph::PatchMgrPtr      m,
														ph::PatchBlock*      b,
														Dyninst::Address     a,
														in::Instruction::Ptr i,
														ph::PatchFunction *f) {
		return new SpPoint(t, m, b, a, i, f);
	}

	ph::Point*
	SpPointMaker::mkEdgePoint(ph::Point::Type    t,
														ph::PatchMgrPtr    m,
														ph::PatchEdge*     e,
														ph::PatchFunction* f) {
		return new SpPoint(t, m, e, f);
	}

	// ------------------------------------------------------------------- 
	// CFGMaker
	// -------------------------------------------------------------------
	ph::PatchFunction*
	SpCFGMaker::makeFunction(pe::Function* f,
													 ph::PatchObject* obj) {
		return new SpFunction(f, obj);
	}

	ph::PatchFunction*
	SpCFGMaker::copyFunction(ph::PatchFunction* f,
													 ph::PatchObject* obj) {
		return new SpFunction(f, obj);
	}

	ph::PatchBlock*
	SpCFGMaker::makeBlock(pe::Block* b,
												ph::PatchObject* obj) {
		return new SpBlock(b, obj);
	}

	ph::PatchBlock*
	SpCFGMaker::copyBlock(ph::PatchBlock* b,
												ph::PatchObject* obj) {
		return new SpBlock(b, obj);
	}

	ph::PatchEdge*
	SpCFGMaker::makeEdge(pe::Edge* e,
											 ph::PatchBlock* s,
											 ph::PatchBlock* t,
											 ph::PatchObject* o) {
		return (new SpEdge(e,
											 s ? s : o->getBlock(e->src()),
											 t ? t : o->getBlock(e->trg())));
	}

	ph::PatchEdge*
	SpCFGMaker::copyEdge(ph::PatchEdge* e,
											 ph::PatchObject* o) {
		return (new SpEdge(e,
											 o->getBlock(e->source()->block()),
											 o->getBlock(e->target()->block())));
	}

}
