#include "Point.h"
#include "PatchCFG.h"
#include "SpInstrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"

using sp::SpInstrumenter;
using Dyninst::PatchAPI::CommandList;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Snippet;

SpInstrumenter::ptr SpInstrumenter::create(Dyninst::PatchAPI::AddrSpacePtr as) {
  return ptr(new SpInstrumenter(as));
}

SpInstrumenter::SpInstrumenter(Dyninst::PatchAPI::AddrSpacePtr as)
  : Dyninst::PatchAPI::Instrumenter(as) {
}

bool SpInstrumenter::run() {
  sp_debug("CODE GEN - Start instrumentation and generate binary");

  for (CommandList::iterator c = user_commands_.begin(); c != user_commands_.end(); c++) {
    PushBackCommand::Ptr command = DYN_CAST(PushBackCommand, *c);
    if (command) {
      InstancePtr instance = command->instance();
      Point* pt = instance->point();
      Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
      SpSnippet::ptr sp_snip = snip->rep();
      char* blob = sp_snip->blob();
    }
  }
}
