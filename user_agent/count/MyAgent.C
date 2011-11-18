#include "SpInc.h"
#include <sys/resource.h>


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

void print_before(Point* pt) {
  //  PatchFunction* f = sp::callee(pt);
  //  sp_print("PRINT_HEAD: %s", f->name().c_str());
  /*  struct rlimit core_limit;
  core_limit.rlim_cur = RLIM_INFINITY;
  core_limit.rlim_max = RLIM_INFINITY;
  if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
    sp_perror("ERROR: failed to setup core dump ability\n");
  }
*/
  callcount++;
  sp::propel(pt);
}

void print_after(Point* pt) {
}

void segv_handler(int num) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::create();
  sp::SpParser::ptr parser = sp::SpParser::create();
  sp::SyncEvent::ptr event = sp::SyncEvent::create();

  agent->set_parser(parser);
  agent->set_init_before("print_before");
  //agent->set_directcall_only(true);
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
  printf("# of calls: %ul\n", callcount);
}
