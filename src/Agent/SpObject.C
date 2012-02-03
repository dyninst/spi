#include "SpObject.h"
#include "SpAgentCommon.h"

namespace sp {

  // Get the object's name
  std::string SpObject::name() {
    if (name_.size() > 0) return name_;

		pe::SymtabCodeSource* cs = (pe::SymtabCodeSource*)co()->cs();
		sb::Symtab* sym = cs->getSymtabObject();
    if (sym) name_ = sym->name();
    return name_;
  }
}
