#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "Instrumenter.h"
#include "SpCommon.h"

namespace sp {
class JumpInstrumenter : public Dyninst::PatchAPI::Instrumenter {
  public:
    static JumpInstrumenter* create(Dyninst::PatchAPI::AddrSpace* as);
    virtual bool run();

  protected:
    JumpInstrumenter(Dyninst::PatchAPI::AddrSpace*);
    bool install(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size);
};

}

#endif /* _SPINSTRUMENTER_H_ */
