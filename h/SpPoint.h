#ifndef _SPPOINT_H_
#define _SPPOINT_H_

#include "Point.h"

namespace sp {
class SpPoint : public Dyninst::PatchAPI::Point {
  public:
    SpPoint(Dyninst::PatchAPI::Point::Type t,
            Dyninst::PatchAPI::PatchMgrPtr m,
            Dyninst::PatchAPI::PatchFunction *f)
      : Dyninst::PatchAPI::Point(t,m,f), propagated_(false), instrumented_(false) {
    }
    SpPoint(Dyninst::PatchAPI::Point::Type t,
            Dyninst::PatchAPI::PatchMgrPtr m,
            Dyninst::PatchAPI::PatchFunction *f,
            Dyninst::PatchAPI::PatchBlock *b)
      : Dyninst::PatchAPI::Point(t,m,f,b), propagated_(false), instrumented_(false) {
    }
    SpPoint(Dyninst::PatchAPI::Point::Type t,
            Dyninst::PatchAPI::PatchMgrPtr m,
            Dyninst::PatchAPI::PatchBlock *b,
            Dyninst::PatchAPI::PatchFunction *f)
      : Dyninst::PatchAPI::Point(t,m,b,f), propagated_(false), instrumented_(false) {
    }
    SpPoint(Dyninst::PatchAPI::Point::Type t,
            Dyninst::PatchAPI::PatchMgrPtr m,
            Dyninst::PatchAPI::PatchBlock *b,
            Dyninst::Address a,
            Dyninst::InstructionAPI::Instruction::Ptr i,
            Dyninst::PatchAPI::PatchFunction *f)
      : Dyninst::PatchAPI::Point(t,m,b,a,i,f), propagated_(false), instrumented_(false) {
    }
    SpPoint(Dyninst::PatchAPI::Point::Type t,
            Dyninst::PatchAPI::PatchMgrPtr m,
            Dyninst::PatchAPI::PatchEdge *e,
            Dyninst::PatchAPI::PatchFunction *f)
      : Dyninst::PatchAPI::Point(t,m,e,f), propagated_(false), instrumented_(false) {
    }

    void set_propagated(bool b) { propagated_ = b; }
    bool propagated() { return propagated_; }

    void set_instrumented(bool b) { instrumented_ = b; }
    bool instrumented() { return instrumented_; }

  protected:
    bool propagated_;
    bool instrumented_;
};


}

#endif /* _SPPOINT_H_ */
