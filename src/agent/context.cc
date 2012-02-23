#include "injector/injector.h"

#include "agent/addr_space.h"
#include "agent/point.h"
#include "agent/parser.h"
#include "agent/propeller.h"
#include "agent/context.h"
#include "common/utils.h"

#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"

namespace sp {

	extern SpParser::ptr g_parser;
  extern SpLock* g_propel_lock;
  extern SpAddrSpace* g_as;

  SpContext::SpContext(SpPropeller::ptr p,
											 SpParser::ptr parser) {
    init_propeller_ = p;
    parser_ = parser;
    ipc_mgr_ = NULL;
    directcall_only_ = false;
    allow_ipc_ = false;

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
    ret->init_entry_ = (void*)g_parser->get_func_addr(init_entry);
    assert(ret->init_entry_);
    ret->init_exit_ = (void*)g_parser->get_func_addr(init_exit);
    ret->init_entry_name_ = init_entry;
    ret->init_exit_name_ = init_exit;
    ret->wrapper_entry_ =
			(void*)parser->get_func_addr("wrapper_entry");
    ret->wrapper_exit_ =
			(void*)parser->get_func_addr("wrapper_exit");
    return ret;
  }

  // Get the first instrumentable function.
  // Here, an instrumentable function should fulfill all of the following
  // requirements:
  //  1. it should be resovled by the parser.
  //  2. it should not be from some well known system libraries
  void
  SpContext::get_callstack(FuncSet* call_stack) {
    long pc, sp, bp;
    parser_->get_frame(&pc, &sp, &bp);
    sp_debug("GET FRAME - pc: %lx, sp: %lx, bp: %lx", pc, sp, bp);
    std::vector<sk::Frame> stackwalk;
		sk::Walker *walker = sk::Walker::newWalker();
		sk::Frame* f = sk::Frame::newFrame(pc, sp, bp, walker);
    walker->walkStackFromFrame(stackwalk, *f);

		sp_debug("WALKED STACK - %ld function calls found", stackwalk.size());
    for (unsigned i=0; i<stackwalk.size(); i++) {
      string s;
      stackwalk[i].getName(s);
      // dt::Address ra = (dt::Address)stackwalk[i].getRA();

      // Step 1: if the function can be resolved
			// ph::PatchFunction* func = parser_->findFunction(ra);
			ph::PatchFunction* func = parser_->findFunction(s.c_str());
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

  SpContext::~SpContext() {
    delete ipc_mgr_;
    delete g_propel_lock;
    delete g_as;
  }

  void
  SpContext::set_allow_ipc(bool b) {
    allow_ipc_ = b;
    if (b && !ipc_mgr_) {
      ipc_mgr_ = new SpIpcMgr();
    }
  }

}
