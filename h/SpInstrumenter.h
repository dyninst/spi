#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "Instrumenter.h"
#include "SpCommon.h"

namespace sp {

class SpInstrumenter  : public Dyninst::PatchAPI::Instrumenter {
  public:
    typedef dyn_detail::boost::shared_ptr<SpInstrumenter> ptr;
    static ptr create(Dyninst::PatchAPI::AddrSpacePtr as);

    virtual bool run();

  protected:
    SpInstrumenter(Dyninst::PatchAPI::AddrSpacePtr);
    bool install(Dyninst::PatchAPI::Point* point, char* blob);
};

}

#endif /* _SPINSTRUMENTER_H_ */
