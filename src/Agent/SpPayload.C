#include "SpPayload.h"

using sp::SpPayload;
using sp::SpContextPtr;

SpPayload::SpPayload() : blob_(NULL) {
  sp_debug("%s", __FUNCTION__);
}

SpPayload::ptr SpPayload::create() {
  return ptr(new SpPayload);
}

bool SpPayload::operator()(SpContextPtr c) {
  // 1. Print out current function name
  // c->getCurrentFunc();
  sp_print("%s", __FUNCTION__);

  // 2. The default implementation doesn't propel
  // c->propel();

  return true;
}

void* SpPayload::binary() {
  if (!blob_) {
    // save context
    // restore context
  }
  return blob_;
}

