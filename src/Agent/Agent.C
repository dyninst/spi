#include "Agent.h"
#include "Common.h"

using sp::AgentConf;

/* Constructor for AgentConf */
AgentConf::ptr AgentConf::create() {
  return ptr(new AgentConf());
}

AgentConf::AgentConf() {
  sp_debug("%s\n", __FUNCTION__);
}
