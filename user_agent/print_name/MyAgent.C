#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int indent = 0;

void print_head(Point* pt, sp::SpContext* context) {

  SpPayload payload(pt, context);
  PatchFunction* f = payload.callee();
  if (!f) return;
  string callee_name = f->name();

  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sEnter %s", fmt.c_str(), callee_name.c_str());
  ++indent;

  payload.propell();
}

void print_tail(Point* pt, sp::SpContext* context) {
  SpPayload payload(pt, context);
  PatchFunction* f = payload.callee();
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
