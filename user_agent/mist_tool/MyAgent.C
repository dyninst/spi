#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int indent = 0;
int callcount = 0;

//--------------------------------------------
// This is invoked before every function call
//--------------------------------------------
void print_before(SpPoint* pt) {

  //---------------------------------------------
  // For tail call, only print_before is invoked,
  // and ignore print_after
  //---------------------------------------------
  if (pt->tailcall()) {
    sp::propel(pt);
    return;
  }

  //---------------------------------------------
  // Get callee's PatchFunction instance
  //---------------------------------------------
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  //---------------------------------------------
  // Get callee's name
  //---------------------------------------------
  string callee_name = f->name();

  //---------------------------------------------
  // Count the number of invocations
  //---------------------------------------------
  callcount ++;

  //---------------------------------------------
  // Control the indentation, and print out call
  // graph
  //---------------------------------------------
  string fmt;
  for (int i = 0; i < indent; i++)
    fmt += ' ';
  sp_print("%sEnter %s %d @ %lx", fmt.c_str(), callee_name.c_str(), callcount, pt->block()->last());
  ++indent;

  //------------------------------------------
  // Propel to next point (next function call
  //------------------------------------------
  sp::propel(pt);
}

//--------------------------------------------
// This is invoked after every function call
//--------------------------------------------
void print_after(Point* pt) {

  //---------------------------------------------
  // Get callee's PatchFunction instance
  //---------------------------------------------
  PatchFunction* f = sp::callee(pt);
  if (!f) return;

  //---------------------------------------------
  // Get callee's name
  //---------------------------------------------
  string callee_name = f->name();

  //---------------------------------------------
  // Control the indentation, and print out call
  // graph
  //---------------------------------------------
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
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  agent->set_init_event(event);
  agent->set_init_before("print_before");
  agent->set_init_after("print_after");
  agent->go();
}
