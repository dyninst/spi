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

SpContext::SpContext(SpPropeller::ptr p,
                     SpParser::ptr parser) {
  init_propeller_ = p;
  parser_ = parser;
  ipc_mgr_ = NULL;
  allow_ipc_ = false;

  // Parsing the entire code
  parse();

  init_well_known_libs();
}

SpContext*
SpContext::create(SpPropeller::ptr propeller,
                  string init_before,
                  string init_after,
                  SpParser::ptr parser) {
  SpContext* ret = new SpContext(propeller,
                                 parser);
  assert(ret);
  ret->init_before_ = (void*)ret->parser()->get_func_addr(init_before);
  ret->init_after_ = (void*)ret->parser()->get_func_addr(init_after);
  ret->init_before_name_ = init_before;
  ret->init_after_name_ = init_after;
  ret->wrapper_before_ = (void*)ret->parser()->get_func_addr("wrapper_before");
  ret->wrapper_after_ = (void*)ret->parser()->get_func_addr("wrapper_after");
  return ret;
}


void
SpContext::init_well_known_libs() {
  well_known_libs_.push_back("libc-");
  well_known_libs_.push_back("libm-");
  well_known_libs_.push_back("ld-");
  well_known_libs_.push_back("libdl-");
  well_known_libs_.push_back("libstdc++");
  well_known_libs_.push_back("libgcc");
  well_known_libs_.push_back("libagent.so");
  well_known_libs_.push_back("libpthread-");

  if (parser_->injected())
    well_known_libs_.push_back(parser_->get_agent_name());
}

// Get the first instrumentable function.
// Here, an instrumentable function should fulfill all of the following requirements:
// 1. it should be resovled by the parser.
// 2. it should not be from some well known system libraries
void 
SpContext::get_callstack(CallStack* call_stack) {
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

    // Step 2: push this function
#ifndef SP_RELEASE
    sp_debug("FOUND - Function %s is in the call stack", s.c_str());
#endif

    call_stack->push_back(func);
  }
}

void
SpContext::parse() {
#ifndef SP_RELEASE
  sp_debug("START PARSING - start parsing binary code");
#endif
  mgr_ = parser_->parse();
#ifndef SP_RELEASE
  sp_debug("FINISH PARSING - finish parsing binary code");
#endif
}

bool
SpContext::is_well_known_lib(string lib) {
  for (unsigned i = 0; i < well_known_libs_.size(); i++) {
    if (lib.find(well_known_libs_[i]) != string::npos) return true;
  }
  return false;
}


void
SpContext::restore() {
  // Restore trap handler
  sigaction(SIGTRAP, &old_act_, NULL);
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

