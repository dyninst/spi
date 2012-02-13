#include "SpObject.h"
#include "SpAgentCommon.h"

namespace sp {

  // Get the object's name
  std::string SpObject::name() {
		assert(symtab_);
    return symtab_->name();
  }

	bool
	SpObject::serialize(const char* target_dir,
											SpObject* obj) {

		return true;
	}

	bool
	SpObject::deserialize(const char* target_dir,
												const char* target_obj,
												SpObject* obj) {
		return true;
	}

}
