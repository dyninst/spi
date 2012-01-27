#include "SpContext.h"
#include "SpPropeller.h"
#include "SpParser.h"
#include "SpUtils.h"

using sk::Frame;
using sk::Walker;

using sb::Symtab;

using sp::SpParser;
using sp::SpContext;
using sp::SpPropeller;

using ph::Point;
using ph::PatchMgr;
using ph::AddrSpace;
using ph::PatchObject;
using ph::PatchMgrPtr;
using ph::PatchFunction;

using pe::SymtabCodeSource;

SpContext::SpContext(SpPropeller::ptr p, SpParser::ptr parser) {
  init_propeller_ = p;
  parser_ = parser;
  ipc_mgr_ = NULL;
	directcall_only_ = false;
  allow_ipc_ = false;
	trap_only_ = false;

  // Parsing the entire code
  parser_->parse();
}

SpContext*
SpContext::create(SpPropeller::ptr propeller,
                  string init_entry,
                  string init_exit,
                  SpParser::ptr parser) {
  SpContext* ret = new SpContext(propeller,
                                 parser);
  assert(ret);
  ret->init_entry_ = (void*)ret->parser()->get_func_addr(init_entry);
  ret->init_exit_ = (void*)ret->parser()->get_func_addr(init_exit);
  ret->init_entry_name_ = init_entry;
  ret->init_exit_name_ = init_exit;
  ret->wrapper_entry_ = (void*)ret->parser()->get_func_addr("wrapper_entry");
  ret->wrapper_exit_ = (void*)ret->parser()->get_func_addr("wrapper_exit");
  return ret;
}

// Get the first instrumentable function.
// Here, an instrumentable function should fulfill all of the following requirements:
// 1. it should be resovled by the parser.
// 2. it should not be from some well known system libraries
void
SpContext::get_callstack(FuncSet* call_stack) {
  long pc, sp, bp;
  parser_->get_frame(&pc, &sp, &bp);
  sp_debug("GET FRAME - pc: %lx, sp: %lx, bp: %lx", pc, sp, bp);
  std::vector<Frame> stackwalk;
  Walker *walker = Walker::newWalker();
  Frame* f = Frame::newFrame(pc, sp, bp, walker);
  walker->walkStackFromFrame(stackwalk, *f);
  for (unsigned i=0; i<stackwalk.size(); i++) {
    string s;
    stackwalk[i].getName(s);
    dt::Address ra = (dt::Address)stackwalk[i].getRA();

    // Step 1: if the function can be resolved
    PatchFunction* func = parser_->findFunction(ra);
    if (!func) {
#ifndef SP_RELEASE
      sp_debug("SKIPPED - Function %s cannot be resolved", s.c_str());
#endif
      continue;
    }

    // Step 2: add this function
#ifndef SP_RELEASE
    sp_debug("FOUND - Function %s is in the call stack", s.c_str());
#endif
    call_stack->insert(func);
  }
}

PatchFunction*
SpContext::callee(Point* pt) {
  return parser()->callee(pt, directcall_only_ == false);
}

SpContext::~SpContext() {
  delete ipc_mgr_;
}

void
SpContext::set_allow_ipc(bool b) {
  allow_ipc_ = b;
  if (b && !ipc_mgr_) {
    ipc_mgr_ = new SpIpcMgr();
  }
}

