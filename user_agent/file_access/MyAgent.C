#include "SpAgent.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using namespace Dyninst;
using namespace PatchAPI;

void tmpl_payload(Point* pt, sp::SpContext* context) {

  PatchFunction* f = context->callee(pt);
  if (!f) return;

  string callee_name = f->name();
  sp_print("%s in tmpl_payload", callee_name.c_str());

  sp::Points pts;
  sp::CalleePoints(f, context, pts);
  sp::SpPropeller::ptr p = sp::SpPropeller::create();
  p->go(pts, context, context->init_payload());
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  agent->set_parser(parser);

  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);

  agent->set_init_payload("tmpl_payload");

  agent->go();
}
