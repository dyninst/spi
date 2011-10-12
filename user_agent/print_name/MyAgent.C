#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int indent = 0;

namespace sp {
  extern void payload_start();
  extern void payload_end();
  extern void report_timer();
}
int callcount = 0;
//SpPayload payload(NULL);

void print_head(Point* pt) {
  //sp::payload_start();

  //SpPayload payload(pt);
  //payload.set_point(pt);
  PatchFunction* f = sp::callee(pt);
  if (!f) return;
  string callee_name = f->name();
  callcount ++;

  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sEnter %s %d", fmt.c_str(), callee_name.c_str(), callcount);
  ++indent;

  sp::propel(pt);
  //sp::payload_end();
  //sp::report_timer();
}

void print_tail(Point* pt) {
  // SpPayload payload(pt);
  //  SpPayload* payload = payload_mgr(pt);
  //payload.set_point(pt);

  PatchFunction* f = sp::callee(pt);
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
  //agent->set_init_event(event);
  agent->set_init_head("print_head");
  agent->set_init_tail("print_tail");
  if (getenv("SP_JUMP")) {
    sp_print("jump version");
    agent->set_directcall_only(true);
    agent->set_jump_inst(true);
  } else {
    sp_print("trap version");
    }
  agent->go();
}
