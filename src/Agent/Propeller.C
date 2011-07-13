#include "Propeller.h"

using sp::Propeller;

Propeller::ptr Propeller::create() {
  return ptr(new Propeller);
}


bool Propeller::operator()() {
  return true;
}
