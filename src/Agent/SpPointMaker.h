#ifndef _SPPOINTMAKER_H_
#define _SPPOINTMAKER_H_

#include "SpPoint.h"

namespace sp {
class SpPointMaker : public Dyninst::PatchAPI::PointMaker {
  protected:
    virtual Dyninst::PatchAPI::Point*
    mkFuncPoint(Dyninst::PatchAPI::Point::Type t,
                Dyninst::PatchAPI::PatchMgrPtr m,
                Dyninst::PatchAPI::PatchFunction *f) {
      return new SpPoint(t, m, f);
    }
    virtual Dyninst::PatchAPI::Point*
    mkFuncSitePoint(Dyninst::PatchAPI::Point::Type t,
                    Dyninst::PatchAPI::PatchMgrPtr m,
                    Dyninst::PatchAPI::PatchFunction *f,
                    Dyninst::PatchAPI::PatchBlock *b) {
      return new SpPoint(t, m, f, b);
    }
    virtual Dyninst::PatchAPI::Point*
    mkBlockPoint(Dyninst::PatchAPI::Point::Type t,
                 Dyninst::PatchAPI::PatchMgrPtr m,
                 Dyninst::PatchAPI::PatchBlock *b,
                 Dyninst::PatchAPI::PatchFunction *f) {
      return new SpPoint(t, m, b, f);
    }
    virtual Dyninst::PatchAPI::Point*
    mkInsnPoint(Dyninst::PatchAPI::Point::Type t,
                Dyninst::PatchAPI::PatchMgrPtr m,
                Dyninst::PatchAPI::PatchBlock *b,
                Dyninst::Address a,
                Dyninst::InstructionAPI::Instruction::Ptr i,
                Dyninst::PatchAPI::PatchFunction *f) {
      return new SpPoint(t, m, b, a, i, f);
    }
    virtual Dyninst::PatchAPI::Point*
    mkEdgePoint(Dyninst::PatchAPI::Point::Type t,
                Dyninst::PatchAPI::PatchMgrPtr m,
                Dyninst::PatchAPI::PatchEdge *e,
                Dyninst::PatchAPI::PatchFunction *f) {
      return new SpPoint(t, m, e, f);
    }
};
}

#endif /* _SPPOINTMAKER_H_ */
