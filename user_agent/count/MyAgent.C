
#include "SpInc.h"
#include <sys/resource.h>


using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

unsigned long long callcount = 0;
unsigned long long springcount = 0;
unsigned long long relocblkcount = 0;
unsigned long long relocinsncount = 0;
unsigned long long trapcount = 0;

namespace sp {
  extern void payload_start();
  extern void payload_end();
  extern void report_timer();
}

void print_before(SpPoint* pt) {
  ++callcount;
  switch (pt->install_method()) {
  case SP_TRAP:
    ++trapcount;
    break;
  case SP_RELOC_BLK:
    ++relocblkcount;
    break;
  case SP_RELOC_INSN:
    ++relocinsncount;
    break;
  case SP_SPRINGBOARD:
    ++springcount;
    break;
  default:
    break;
  }
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
  // agent->set_directcall_only(true);
  agent->go();
}

__attribute__((destructor))
void DumpOutput() {
  printf("# of calls: %llu\n", callcount);
  printf("# of springboard calls: %llu\n", springcount);
  printf("# of reloc insn calls: %llu\n", relocinsncount);
  printf("# of reloc blk calls: %llu\n", relocblkcount);
  printf("# of trap calls: %llu\n", trapcount);
}
