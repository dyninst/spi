#include "Payload.h"

using sp::Payload;
using sp::ContextPtr;

Payload::Payload() : blob_(NULL) {
  sp_debug("%s", __FUNCTION__);
}

Payload::ptr Payload::create() {
  return ptr(new Payload);
}

bool Payload::operator()(ContextPtr c) {
  // 1. Print out current function name
  // c->getCurrentFunc();
  sp_print("%s", __FUNCTION__);

  // 2. The default implementation doesn't propel
  // c->propel();

  return true;
}

void* Payload::binary() {
  if (!blob_) {
    // save context
    // restore context
  }
  return blob_;
}

