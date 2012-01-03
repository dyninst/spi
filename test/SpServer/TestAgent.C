#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(SpPoint* pt) {

  PatchFunction* f = callee(pt);
  if (!f) return;

  if (start_tracing()) {
    if (is_ipc_write(pt)) {
      sp_print("Write: %s @ pid=%d w/ addr %lx", f->name().c_str(), getpid(), f->addr());
    }
    else if (is_ipc_read(pt)) {
      sp_print("Read: %s @ pid=%d w/ addr %lx", f->name().c_str(), getpid(), f->addr());
    }
  }
  sp::propel(pt);
}

void test_after(SpPoint* pt) {
  PatchFunction* f = callee(pt);
  if (!f) return;

  if (start_tracing()) {
    if (is_ipc_write(pt)) {
      long size = sp::retval(pt);
      sp_print("Write size: %lu", size);
    }
    else if (is_ipc_read(pt)) {
      long size = sp::retval(pt);
      sp_print("Read size: %lu", size);
    }
  }
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_before("test_before");
  agent->set_init_after("test_after");
  agent->set_ipc(true);
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
}
