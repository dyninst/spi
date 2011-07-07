#include "Common.h"
#include "stdlib.h"

bool sp_debug_flag = false;
DebugConfig debug_config;

DebugConfig::DebugConfig() {
  if (getenv("SP_DEBUG")) sp_debug_flag = true;
}
