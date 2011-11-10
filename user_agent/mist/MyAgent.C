#include "SpInc.h"
#include <stack>

#include "MistChecker.h"
#include "CheckerUtils.h"
#include "Mist.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

class Mist;
extern Mist mist;

// TODO: separate print strings by pid

//--------------------------------------------------------
// Standard self-propelled stuffs
//--------------------------------------------------------
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

  sp_print("=============================================");
  sp_print("========= CHECKING: for each call ===========");
  sp_print("=============================================");
}
