#include "SpPoint.h"
#include "SpObject.h"

namespace sp {

	SpBlock* SpPoint::get_block() const {
		return static_cast<SpBlock*>(block());
	}

	SpObject* SpPoint::get_object() const {
		return static_cast<SpObject*>(obj());
	}

}
