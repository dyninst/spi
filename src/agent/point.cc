#include "agent/point.h"
#include "agent/object.h"

namespace sp {

	SpBlock* SpPoint::GetBlock() const {
		return static_cast<SpBlock*>(block());
	}

	SpObject* SpPoint::GetObject() const {
		return static_cast<SpObject*>(obj());
	}

}
