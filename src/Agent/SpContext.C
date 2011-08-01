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
  sp_debug("SpContext::%s", __FUNCTION__);
  assert(p);
  assert(ip);

  propeller_ = p;
  init_payload_ = ip;
  parser_ = parser;
}

SpContextPtr SpContext::create(SpPropeller::ptr propeller,
                               SpPayload::ptr init_payload,
                               SpParser::ptr parser) {
  sp_debug("SpContext::%s", __FUNCTION__);
  SpContextPtr ret = SpContextPtr(new SpContext(propeller,
                                                init_payload,
                                                parser));
  assert(ret);
  propeller->set_context(ret);
  return ret;
}

bool SpContext::propel(SpPropeller::PointType type,
                       SpPayload::ptr payload) {
  sp_debug("SpContext::%s", __FUNCTION__);
  propeller_->go(type, payload);
}

/* Get the first instrumentable function.
   Here, the instrumentable function is the caller of the first encountered
   system call wrapper. For example, we are currently in the function __A,
   which is called by the system call sleep(), and sleep() is called by main(),
   and main() is called by __start(). So, main() is the first instrumentable function.
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
    sp_debug("In Call Stack - %s in %s w/ offset %lx", s.c_str(), l.c_str(), o);
    // find the first syscall wrapper
  }
}

/* Parse the binary and initialize PatchAPI structures. */
void SpContext::parse() {
  SpParser::PatchObjects& cos = parser_->parse();
  sp_debug("%d PatchObjects created", cos.size());

  PatchObject* exe_obj = parser_->exe_obj();
  assert(exe_obj);

  sp_debug("PatchObject for exe with load address 0x%lx", exe_obj->codeBase());
  AddrSpacePtr as = AddrSpace::create(exe_obj);
  mgr_ = PatchMgr::create(as);

  for (SpParser::PatchObjects::iterator i = cos.begin(); i != cos.end(); i++) {
    if (*i != exe_obj) {
      as->loadObject(*i);
    }
  }
}
