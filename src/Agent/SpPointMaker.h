#ifndef _SPPOINTMAKER_H_
#define _SPPOINTMAKER_H_

#include "SpAgentCommon.h"

/* PatchAPI stuffs: build customized points */
namespace sp {
class SpPointMaker : public ph::PointMaker {
  protected:

    virtual ph::Point*
    mkFuncPoint(ph::Point::Type    t,
                ph::PatchMgrPtr    m,
                ph::PatchFunction* f) {
      return new SpPoint(t, m, f);
    }

    virtual ph::Point*
    mkFuncSitePoint(ph::Point::Type    t,
                    ph::PatchMgrPtr    m,
                    ph::PatchFunction* f,
                    ph::PatchBlock*    b) {
      return new SpPoint(t, m, f, b);
    }

    virtual ph::Point*
    mkBlockPoint(ph::Point::Type    t,
                 ph::PatchMgrPtr    m,
                 ph::PatchBlock*    b,
                 ph::PatchFunction* f) {
      return new SpPoint(t, m, b, f);
    }
    virtual ph::Point*
    mkInsnPoint(ph::Point::Type      t,
                ph::PatchMgrPtr      m,
                ph::PatchBlock*      b,
                Dyninst::Address     a,
                in::Instruction::Ptr i,
                ph::PatchFunction *f) {
      return new SpPoint(t, m, b, a, i, f);
    }
    virtual ph::Point*
    mkEdgePoint(ph::Point::Type    t,
                ph::PatchMgrPtr    m,
                ph::PatchEdge*     e,
                ph::PatchFunction* f) {
      return new SpPoint(t, m, e, f);
    }
};
}

#endif /* _SPPOINTMAKER_H_ */
