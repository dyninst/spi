#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "Instrumenter.h"
#include "SpCommon.h"

namespace sp {

class SpInstrumenter  : public Dyninst::PatchAPI::Instrumenter {
  public:
    static SpInstrumenter* create(Dyninst::PatchAPI::AddrSpace* as);

    virtual bool run();

  protected:
    SpInstrumenter(Dyninst::PatchAPI::AddrSpace*);
    bool install(Dyninst::PatchAPI::Point* point, char* blob);
};

}

#endif /* _SPINSTRUMENTER_H_ */
