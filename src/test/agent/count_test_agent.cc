
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

void* print_entry(PointCallHandle* handle) {
  SpPoint* pt = handle->GetPoint();
  SetSegfaultSignal();

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
  return NULL;
}

void print_exit(sp::PointCallHandle*) {
}

void segv_handler(int num) {
}

AGENT_INIT
void MyAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  agent->SetInitEntry("print_entry");
  agent->SetInitExit("print_exit");
  agent->Go();
}

__attribute__((destructor))
void DumpOutput() {
  fprintf(stderr, "# of calls: %llu\n", callcount);
  fprintf(stderr, "# of springboard calls: %llu (%f%%)\n", springcount, (double)springcount/(double)callcount*100.0);
  fprintf(stderr, "# of reloc insn calls: %llu (%f%%)\n", relocinsncount, (double)relocinsncount/(double)callcount*100.0);
  fprintf(stderr, "# of reloc blk calls: %llu (%f%%)\n", relocblkcount, (double)relocblkcount/(double)callcount*100.0);
  fprintf(stderr, "# of trap calls: %llu (%f%%)\n", trapcount, (double)trapcount/(double)callcount*100.0);
}
