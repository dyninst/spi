#ifndef _SPPOINTMAKER_H_
#define _SPPOINTMAKER_H_

#include "patchAPI/h/CFGMaker.h"

#include "common/common.h"

// PatchAPI stuffs: build customized points and CFG
namespace sp {
  class SpPointMaker : public ph::PointMaker {
  protected:

    virtual ph::Point*
			mkFuncPoint(ph::Point::Type    t,
                  ph::PatchMgrPtr    m,
                  ph::PatchFunction* f);

    virtual ph::Point*
			mkFuncSitePoint(ph::Point::Type    t,
                      ph::PatchMgrPtr    m,
                      ph::PatchFunction* f,
                      ph::PatchBlock*    b);

    virtual ph::Point*
			mkBlockPoint(ph::Point::Type    t,
                   ph::PatchMgrPtr    m,
                   ph::PatchBlock*    b,
                   ph::PatchFunction* f);

    virtual ph::Point*
			mkInsnPoint(ph::Point::Type      t,
                  ph::PatchMgrPtr      m,
                  ph::PatchBlock*      b,
                  Dyninst::Address     a,
                  in::Instruction::Ptr i,
                  ph::PatchFunction *f);

    virtual ph::Point*
			mkEdgePoint(ph::Point::Type    t,
                  ph::PatchMgrPtr    m,
                  ph::PatchEdge*     e,
                  ph::PatchFunction* f);
  };


	class SpCFGMaker : public ph::CFGMaker {
  public:
    SpCFGMaker() {}
    virtual ~SpCFGMaker() {}

    virtual ph::PatchFunction*
			makeFunction(pe::Function* f,
									 ph::PatchObject* obj);

    virtual ph::PatchFunction*
			copyFunction(ph::PatchFunction* f,
									 ph::PatchObject* obj);

    virtual ph::PatchBlock*
			makeBlock(pe::Block* b,
								ph::PatchObject* obj);

    virtual ph::PatchBlock*
			copyBlock(ph::PatchBlock* b,
								ph::PatchObject* obj);

    virtual ph::PatchEdge*
			makeEdge(pe::Edge* e,
							 ph::PatchBlock* s,
							 ph::PatchBlock* t,
							 ph::PatchObject* o);

    virtual ph::PatchEdge*
			copyEdge(ph::PatchEdge* e,
							 ph::PatchObject* o);
	};

}

#endif /* _SPPOINTMAKER_H_ */
