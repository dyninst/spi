#include "SpCFG.h"
#include "SpObject.h"

namespace sp {

// ------------------------------------------------------------------- 
// SpFunction
// -------------------------------------------------------------------

	SpObject* SpFunction::get_object() const {
    return OBJ_CAST(obj());
	}

// ------------------------------------------------------------------- 
// SpBlock
// -------------------------------------------------------------------
	bool SpBlock::save() {

		// Save the call instruction
		orig_call_addr_ = last();
		orig_call_insn_ = getInsn(orig_call_addr_);
    if (!orig_call_insn_) return false;

		// Save the entire block
    char* blk_buf = (char*)start();
    for (unsigned i = 0; i < size(); i++) {
      orig_code_ += (char)blk_buf[i];
    }
		

		return true;
	}

	SpObject* SpBlock::get_object() const {
    return OBJ_CAST(obj());
	}

}
