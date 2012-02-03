#include "SpPoint.h"

namespace sp {

  void SpPoint::set_orig_spring(ph::PatchBlock* blk) {
    assert(blk);
    call_blk_addr_ = blk->start();
    char* blk_buf = (char*)blk->start();
    for (unsigned i = 0; i < blk->size(); i++) {
      orig_call_blk_ += (char)blk_buf[i];
    }
  }

  void SpPoint::set_orig_call_blk(ph::PatchBlock* blk) {
    assert(blk);
    spring_addr_ = blk->start();
    char* blk_buf = (char*)blk->start();
    for (unsigned i = 0; i < blk->size(); i++) {
      orig_spring_ += (char)blk_buf[i];
    }
  }

  bool SpPoint::instrumented() {
		return instrumented_;
	}
}
