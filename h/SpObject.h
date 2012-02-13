#ifndef _SPOBJECT_H_
#define _SPOBJECT_H_

#include "SpAgentCommon.h"

// PatchAPI stuffs.
namespace sp {

  class SpObject : public ph::PatchObject {
  public:

    SpObject(pe::CodeObject* o,
						 dt::Address a,
						 ph::CFGMaker* cm,
             ph::PatchCallback* cb,
						 dt::Address la,
						 sb::Symtab* symtab)
			: ph::PatchObject(o, a, cm, cb),
			load_addr_(la),
			symtab_(symtab) {}

    dt::Address load_addr() const { return load_addr_; }

		// Get this object's name
		std::string name();

		// Get this object's associated symtab
		sb::Symtab* symtab() const { return symtab_; }

		// Serialization / Deserialization
		static bool serialize(const char* target_dir,
						              SpObject* obj);
		static bool deserialize(const char* target_dir,
														const char* target_obj,
														SpObject* obj);
  protected:
    dt::Address load_addr_;
		std::string name_;

		sb::Symtab* symtab_;
};

}

#endif /* _SPOBJECT_H_ */
