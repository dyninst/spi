#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "SpAgentCommon.h"

namespace sp {

class SpObject;

//   Parser is to parse the CFG structures of the mutatee process.
class SpParser : public ph::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<SpParser> ptr;

    virtual ~SpParser();
    static ptr create();

		// The main parsing procedure
    virtual ph::PatchMgrPtr parse();

		// Get the PatchObject that represents the executable
    ph::PatchObject* exe_obj();

		// Get the agent library's name
    string get_agent_name();

		// Get register values to form a stack frame
    void get_frame(long* pc, long* sp, long* bp);

		// Find function by absolute address
    ph::PatchFunction* findFunction(dt::Address addr);

		// Find function by its name
    ph::PatchFunction* findFunction(string name);

		// Get callee from a call point
    ph::PatchFunction* callee(ph::Point* pt,
                              bool       parse_indirect = false);

		// Get function absolute address from function name
    dt::Address get_func_addr(string name);

		// Dump instructions from a buffer
    string dump_insn(void* addr, size_t size);

		// Check if this agent library is injected (true) or is preloaded (false)
    bool injected() const { return injected_; }

		// Check if the library is a dyninst library (lib name is w/o path)
    bool is_dyninst_lib(string lib);

		// Check if the library is a well known library (lib name is w/o path)
    bool is_well_known_lib(string lib);

		// Get SpObject from a PatchFunction, this is actually a static_cast
		SpObject* get_object(ph::PatchFunction* func);

  protected:
    CodeSources code_srcs_;
    CodeObjects code_objs_;
    ph::PatchObject* exe_obj_;
    ph::PatchMgrPtr mgr_;
    bool injected_;
    ucontext_t old_context_;
    std::vector<std::string> dyninst_libs_;
    std::vector<std::string> well_known_libs_;
    bool jump_;
    size_t sp_offset_;  // offset of stack pointer for saved context

    typedef std::map<string,
      ph::PatchFunction*> RealFuncMap;
    RealFuncMap real_func_map_;

    string agent_name_;
    string exe_name_;

    SpParser();
		void init_well_known_libs();
    void init_dyninst_libs();
};

}

#endif /* SP_PARSER_H_ */
