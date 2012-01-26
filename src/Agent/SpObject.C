#include "SpObject.h"
#include "SpAgentCommon.h"

using sb::Symtab;

using pe::CodeObject;
using pe::SymtabCodeSource;

namespace sp {

	// Get the object's name
	std::string SpObject::name() {
		if (name_.size() > 0) return name_;

		SymtabCodeSource* cs = (SymtabCodeSource*)co()->cs();
		Symtab* sym = cs->getSymtabObject();
		if (sym) name_ = sym->name();
		return name_;
	}
}
