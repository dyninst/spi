#ifndef _SPOBJECT_H_
#define _SPOBJECT_H_

#include "SpAgentCommon.h"

/* PatchAPI stuffs. */
namespace sp {
  class SpObject : public Dyninst::PatchAPI::PatchObject {
  public:
  SpObject(Dyninst::ParseAPI::CodeObject* o, Dyninst::Address a,
           Dyninst::PatchAPI::CFGMaker* cm, Dyninst::PatchAPI::PatchCallback* cb,
           Dyninst::Address la)
    : Dyninst::PatchAPI::PatchObject(o, a, cm, cb), load_addr_(la) {}
    Dyninst::Address load_addr() const { return load_addr_; }
  protected:
    Dyninst::Address load_addr_;
};

}

#endif /* _SPOBJECT_H_ */
