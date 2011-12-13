#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

void test_before(SpPoint* pt) {

  PatchFunction* f = callee(pt);
  if (!f) return;
  // sp_print("func - %s", f->name().c_str());

  /*
  if (start_tracing()) {
    if (f->name().compare("write") == 0 ||
	f->name().compare("read") == 0) {
      ArgumentHandle h;
      int* fd = (int*)sp::pop_argument(pt, &h, sizeof(int));
      if (is_ipc(*fd)) {
	sp_print("SendBefore: %s @ pid=%d w/ addr %lx", f->name().c_str(), getpid(), f->addr());
      }
    }
  }
*/
  std::vector<string> blacklist;
  blacklist.push_back("free");
  blacklist.push_back("malloc");
  blacklist.push_back("getenv");
  blacklist.push_back("hash");
  blacklist.push_back("find_variable");

  for (int i = 0; i< blacklist.size(); i++) {
    if (f->name().find(blacklist[i]) != std::string::npos) return;
  }

  sp::propel(pt);
}

void test_after(SpPoint* pt) {
  /*
  PatchFunction* f = callee(pt);
  if (!f) return;

  if (start_tracing()) {
    if (f->name().compare("write") == 0) {
      long count = sp::retval(pt);
      sp_print("SendAfter: %s @ pid=%d w/ addr %lx w/ count %d", f->name().c_str(), getpid(), f->addr(), count);
    }
  }
  */
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
