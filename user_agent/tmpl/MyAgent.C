#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int indent = 0;
int callcount = 0;

void print_head(SpPoint* pt) {

if (pt->tailcall()) {
    sp::propel(pt);
    return;
  }

  PatchFunction* f = sp::callee(pt);
  if (!f) return;
  string callee_name = f->name();
  callcount ++;

  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sEnter %s %d @ %lx", fmt.c_str(), callee_name.c_str(), callcount, pt->block()->last());
  ++indent;

  sp::propel(pt);
}

void print_tail(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  if (!f) return;
  string callee_name = f->name();

  --indent;
  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sLeave %s %d @ %lx", fmt.c_str(), callee_name.c_str(), callcount, pt->block()->last());
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();

  sp::SpParser::ptr parser = sp::SpParser::create();
  agent->set_parser(parser);

  sp::SyncEvent::ptr event = sp::SyncEvent::create();
  agent->set_init_event(event);

  agent->set_init_head("print_head");
  agent->set_init_tail("print_tail");

  //agent->set_directcall_only(true);
  agent->go();
}
