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
using sp::SpPropeller;
using sp::SpParser;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::ParseAPI::SymtabCodeSource;
using Dyninst::SymtabAPI::Symtab;

SpContext::SpContext(SpPropeller::ptr p,
                     SpParser::ptr parser) {
  init_propeller_ = p;
  parser_ = parser;
  parse();
  init_well_known_libs();
}

SpContext* SpContext::create(SpPropeller::ptr propeller,
                               string init_payload,
                               SpParser::ptr parser) {
  SpContext* ret = new SpContext(propeller,
                                 parser);
  assert(ret);
  ret->init_payload_ = (void*)ret->parser()->get_func_addr(init_payload);
  return ret;
}


void SpContext::init_well_known_libs() {
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

/* Get the first instrumentable function.
   Here, an instrumentable function should fulfill all of the following requirements:
   1. it should be resovled by the parser.
   2. it should not be from some well known system libraries, including
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

    // Step 1: if the function is in a well known library
    if (is_well_known_lib(l)) {
      sp_debug("SKIPPED - Function %s is in well known lib", s.c_str());
      continue;
    }

    // Step 2: if the function can be resolved
    PatchFunction* func = parser_->findFunction(s);//parser_->findFunction(stackwalk[i].getRA());
    if (!func) {
      sp_debug("SKIPPED - Function %s cannot be resolved", s.c_str());
      continue;
    }

    // Step 3: return this function
    sp_debug("FOUND - Function %s is the first instrumentable function", s.c_str());
    return func;
  }
  return NULL;
}

void SpContext::parse() {
  mgr_ = parser_->parse();

  PatchFunction* setcontext_f = parser()->findFunction("setcontext", false);
  if (!setcontext_f) sp_perror("FATAL - Cannot find setcontext()");
  setcontext_func_ = setcontext_f->addr();
  if (!setcontext_func_) sp_perror("FATAL - Cannot find setcontext()'s address");
  PatchFunction* getcontext_f = parser()->findFunction("getcontext", false);
  if (!getcontext_f) sp_perror("FATAL - Cannot find getcontext()");
  getcontext_func_ = getcontext_f->addr();
  if (!getcontext_func_) sp_perror("FATAL - Cannot find getcontext()'s address");
}

bool SpContext::is_well_known_lib(string lib) {
  for (int i = 0; i < well_known_libs_.size(); i++) {
    if (lib.find(well_known_libs_[i]) != string::npos) return true;
  }
  return false;
}

/*
bool SpContext::is_instrumentable_func(PatchFunction* func) {
  // if the function is in a well known library
  AddrSpacePtr as = mgr_->as();
  PatchObject* obj = func->object();
  SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
  Symtab* sym = cs->getSymtabObject();
  if (is_well_known_lib(sym->name())) {
    sp_debug("NO INST - Function %s in a well known lib %s",
             func->name().c_str(), sp_filename(sym->name().c_str()));
    return false;
  }
  sp_debug("INST - Function %s instrumentable", func->name().c_str());
  return true;
}
*/

void SpContext::restore() {
  // Restore trap handler
  sigaction(SIGTRAP, &old_act_, NULL);
}
