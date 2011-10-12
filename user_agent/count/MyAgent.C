#include "SpInc.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

int callcount = 0;
namespace sp {
  extern void payload_start();
  extern void payload_end();
  extern void report_timer();
}
int indent = 0; 
//sp::SpPayload* payload = NULL;
void print_head(Point* pt) {
  //sp::payload_start();
  // SpPayload payload(pt);
  //SpPayload* payload = payload_mgr(pt);
  callcount++;
  //sp_print("%d", callcount);
  //indent++;
  //int max = 3093050;
  //if (callcount < (max/100000))
  //payload->propell();
  sp::propel(pt);
  //sp::payload_end();
  //sp::report_timer();
}

void print_tail(Point* pt) {
  //indent--;
}

void segv_handler(int num) {
  //sp_print("dump maps");
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  //agent->set_init_event(event);
  agent->set_init_head("print_head");
  // agent->set_init_tail("print_tail");
  if (getenv("SP_JUMP")) {
    agent->set_directcall_only(true);
    agent->set_jump_inst(true);
    sp_print("Jump-based");
  } else {
    agent->set_directcall_only(true);
    sp_print("Trap-based");
  }
 // }
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
  printf("# of calls: %d\n", callcount);
}
