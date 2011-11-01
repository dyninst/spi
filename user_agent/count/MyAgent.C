#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

unsigned long long callcount = 0;
namespace sp {
  extern void payload_start();
  extern void payload_end();
  extern void report_timer();
}
int indent = 0; 

void print_head(Point* pt) {
  PatchFunction* f = sp::callee(pt);
  sp_print("PRINT_HEAD: %s", f->name().c_str());
  callcount++;
  sp::propel(pt);
}

void print_tail(Point* pt) {
}

void segv_handler(int num) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  agent->set_init_head("print_head");
  //agent->set_directcall_only(true);
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
  printf("# of calls: %ul\n", callcount);
}
