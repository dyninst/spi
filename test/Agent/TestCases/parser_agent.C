#include "test_case.h"

__attribute__((constructor))
void init_parser() {
  dprint("AGENT: init_parser");
}
