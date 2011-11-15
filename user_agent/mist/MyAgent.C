#include <stack>

#include "SpInc.h"
#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

Mist mist;
void mist_head(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  CheckerUtils::push(f);
  mist.run(pt, f);
  if (pt->tailcall()) {
    CheckerUtils::pop();
  }
  sp::propel(pt);
}

void mist_tail(SpPoint* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;
  mist.post_run(pt, f);
  CheckerUtils::pop();
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);
  agent->set_init_head("mist_head");
  agent->set_init_tail("mist_tail");
  agent->go();
}
