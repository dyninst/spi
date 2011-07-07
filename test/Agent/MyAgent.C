#include <stdio.h>
#include "Agent.h"

using sp::AgentConf;

__attribute__((constructor))
void hello() {
  printf("*********************** Hello *******************************!\n");

  AgentConf::ptr conf = AgentConf::create();
}
