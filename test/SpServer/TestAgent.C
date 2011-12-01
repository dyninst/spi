#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(SpPoint* pt) {
  PatchFunction* f = callee(pt);
  if (!f) return;

  if (start_tracing()) {
    if (f->name().compare("write") == 0 ||
	f->name().compare("read") == 0) {
      ArgumentHandle h;
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      if (is_ipc(*fd)) {
	sp_print("Send: %s @ pid=%d", f->name().c_str(), getpid());
      }
    }
  }
  sp::propel(pt);
}

void test_after(SpPoint* pt) {
  /*
  PatchFunction* f = callee(pt);
  if (!f) return;
  */
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  agent->set_init_before("test_before");
  agent->set_init_after("test_after");
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
}
