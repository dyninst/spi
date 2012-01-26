#ifndef _SPOBJECT_H_
#define _SPOBJECT_H_

#include "SpAgentCommon.h"

// PatchAPI stuffs.
namespace sp {

  class SpObject : public ph::PatchObject {
  public:

    SpObject(pe::CodeObject* o, dt::Address a, ph::CFGMaker* cm,
             ph::PatchCallback* cb, dt::Address la)
    : ph::PatchObject(o, a, cm, cb), load_addr_(la) {}

    dt::Address load_addr() const { return load_addr_; }

		// Get this object's name
		std::string name();
  protected:
    dt::Address load_addr_;
		std::string name_;
};

}

#endif /* _SPOBJECT_H_ */
