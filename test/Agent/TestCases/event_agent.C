#include "test_case.h"

__attribute__((constructor))
void init_event() {
  dprint("AGENT: init_event");
}
