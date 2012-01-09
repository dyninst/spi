#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(SpPoint* pt) {

  PatchFunction* f = callee(pt);
  if (!f) return;
	// sp_print("before func: %s @ pid=%d", f->name().c_str(), getpid());

  if (start_tracing()) {
    if (is_ipc_write(pt)) {
      fprintf(stderr, "Write: %s @ pid=%d w/ addr %lx\n", f->name().c_str(), getpid(), f->addr());
    }
    else if (is_ipc_read(pt)) {
      fprintf(stderr, "Read: %s @ pid=%d w/ addr %lx\n", f->name().c_str(), getpid(), f->addr());
    }
  }
  sp::propel(pt);
}

void test_after(SpPoint* pt) {
  PatchFunction* f = callee(pt);
  if (!f) return;
	// sp_print("after func: %s @ pid=%d", f->name().c_str(), getpid());

  if (start_tracing()) {
    if (is_ipc_write(pt)) {
      long size = sp::retval(pt);
      fprintf(stderr, "Write size: %lu @ pid=%d\n", size, getpid());
    }
    else if (is_ipc_read(pt)) {
      long size = sp::retval(pt);
      fprintf(stderr, "Read size: %lu @ pid=%d\n", size, getpid());
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
