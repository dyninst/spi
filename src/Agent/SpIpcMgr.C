#include "SpIpcMgr.h"

namespace sp {

// Instrumentation cannot propagate because:
// 1. libagent.so is injected by SpServer and the process?? 
// 2. 
bool SpIpcMgr::can_propagate() {
  return true;
}

}
