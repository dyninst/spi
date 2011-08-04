#include "SpContext.h"
#include "SpPropeller.h"
#include "SpParser.h"
#include "PatchMgr.h"
#include "PatchCFG.h"

#include "frame.h"
#include "walker.h"
using Dyninst::Stackwalker::Walker;
using Dyninst::Stackwalker::Frame;


using sp::SpContext;
using sp::SpContextPtr;
using sp::SpPropeller;
using sp::SpPayload;
using sp::SpParser;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::AddrSpace;

SpContext::SpContext(SpPropeller::ptr p,
                     SpPayload::ptr ip,
                     SpParser::ptr parser) {
  assert(p);
  assert(ip);

  propeller_ = p;
  init_payload_ = ip;
  parser_ = parser;
}

SpContextPtr SpContext::create(SpPropeller::ptr propeller,
                               SpPayload::ptr init_payload,
                               SpParser::ptr parser) {
  SpContextPtr ret = SpContextPtr(new SpContext(propeller,
                                                init_payload,
                                                parser));
  assert(ret);

  return ret;
}

bool SpContext::propel(int type,
                       SpPayload::ptr payload) {
  propeller_->go(type, payload);
}

/* Get the first instrumentable function.
   Here, an instrumentable function should fulfill all of the following requirements:
   1. it should be resovled by the parser.
   2. it should not be a system call.
   3. it should not be from some well known system libraries, including
      3.1, libc
      3.2, ld
      3.3, libm
      3.4, libpthread
      3.5, libgcc
      3.6, libstdc++
 */
PatchFunction* SpContext::get_first_inst_func() {
  std::vector<Frame> stackwalk;
  Walker *walker = Walker::newWalker();
  walker->walkStack(stackwalk);
  for (unsigned i=0; i<stackwalk.size(); i++) {
    string s;
    stackwalk[i].getName(s);
    string l;
    Dyninst::Offset o;
    void* symobj;
    stackwalk[i].getLibOffset(l, o, symobj);
    sp_debug("STACKWALK - %s in library %s with offset %lx",
             s.c_str(), sp_filename(l.c_str()), o);
    // find the first syscall wrapper
  }
}

/* Parse the binary and initialize PatchAPI structures. */
void SpContext::parse() {
  mgr_ = parser_->parse();
}
