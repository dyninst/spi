#include "SpAgent.h"
#include "SpContext.h"
#include "SpNextPoints.h"

using namespace Dyninst;
using namespace PatchAPI;

int indent = 0;

void print_head(Point* pt, sp::SpContext* context) {

  PatchFunction* f = context->callee(pt);
  if (!f) return;
  string callee_name = f->name();

  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sEnter %s", fmt.c_str(), callee_name.c_str());
  ++indent;

  sp::Points pts;
  sp::CalleePoints(f, context, pts);
  sp::SpPropeller::ptr p = sp::SpPropeller::create();
  p->go(pts, context, context->init_head(), context->init_tail());
}

void print_tail(Point* pt, sp::SpContext* context) {
  PatchFunction* f = context->callee(pt);
  if (!f) return;
  string callee_name = f->name();

  --indent;
  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sLeave %s", fmt.c_str(), callee_name.c_str());

}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  agent->set_init_event(event);
  agent->set_init_head("print_head");
  agent->set_init_tail("print_tail");

  agent->go();
}
