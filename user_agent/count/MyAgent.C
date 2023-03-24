
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

void print_before(PointCallHandle* handle) {
  SpPoint* pt = handle->GetPoint();
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
  sp::Propel(pt);
}

void print_after(PointCallHandle* handle) {
}

void segv_handler(int num) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  sp::SpParser::ptr parser = sp::SpParser::Create();
  sp::SyncEvent::ptr event = sp::SyncEvent::Create();

  agent->SetParser(parser);
  agent->SetInitEntry("print_before");
  agent->SetInitExit("print_after");
  agent->Go();
}

__attribute__((destructor))
void DumpOutput() {
  printf("# of calls: %llu\n", callcount);
  printf("# of springboard calls: %llu\n", springcount);
  printf("# of reloc insn calls: %llu\n", relocinsncount);
  printf("# of reloc blk calls: %llu\n", relocblkcount);
  printf("# of trap calls: %llu\n", trapcount);
}
