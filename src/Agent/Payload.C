#include "Payload.h"

using sp::Payload;

Payload::ptr Payload::create() {
  return ptr(new Payload);
}

bool Payload::operator()() {
  return true;
}
