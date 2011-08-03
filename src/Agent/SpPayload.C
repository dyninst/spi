#include "SpPayload.h"

using sp::SpPayload;
using sp::SpContextPtr;

SpPayload::SpPayload()  {
  sp_debug("%s", __FUNCTION__);
}

SpPayload::ptr SpPayload::create() {
  return ptr(new SpPayload);
}

