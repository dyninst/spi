#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "SpEvent.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

namespace sp {
void SpContext::init_well_known_libs() {
  well_known_libs_.push_back("libc-");
  well_known_libs_.push_back("libm-");
  well_known_libs_.push_back("ld-");
  well_known_libs_.push_back("libdl-");
  well_known_libs_.push_back("libstdc++");
  well_known_libs_.push_back("libgcc");
}

char call_payload_code[] = {
  0x11
};

char* SpSnippet::blob() {
  sp_debug("BLOB - Invoke payload %lx with parameters 1) context %lx and 2) func %s",
           payload_, context_.get(), func_->name().c_str());
  return call_payload_code;
}

}
