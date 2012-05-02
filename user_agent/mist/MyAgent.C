#include <stack>

#include "SpInc.h"
#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

Mist mist;
void mist_before(SpPoint* pt) {
  PatchFunction* f = sp::Callee(pt);
  if (!f) return;

  CheckerUtils::push(f);
  mist.run(pt, f);
  if (pt->tailcall()) {
    CheckerUtils::pop();
  }
  sp::Propel(pt);
}

void mist_after(SpPoint* pt) {
  PatchFunction* f = sp::Callee(pt);
  if (!f) return;
  mist.post_run(pt, f);
  CheckerUtils::pop();
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  sp::SyncEvent::ptr event = sp::SyncEvent::Create();
  agent->SetInitEvent(event);
  agent->SetInitEntry("mist_before");
  agent->SetInitExit("mist_after");
  agent->Go();
}
