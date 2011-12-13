#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "SpAgentCommon.h"

namespace sp {

/*
   Parser is to parse the CFG structures of the mutatee process.
 */
class SpParser : public ph::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<SpParser> ptr;
    virtual ~SpParser();
    static ptr create();

    typedef std::vector<pe::CodeObject*> CodeObjects;
    typedef std::vector<ph::PatchObject*> PatchObjects;
    virtual ph::PatchMgrPtr parse();
    ph::PatchObject* exe_obj();
    string get_agent_name();
    void get_frame(long* pc, long* sp, long* bp);

    ph::PatchFunction* findFunction(dt::Address addr);
    ph::PatchFunction* findFunction(string name, bool skip = true);
    ph::PatchFunction* callee(ph::Point* pt,
                              bool       parse_indirect = false);

    dt::Address get_func_addr(string name);
    string dump_insn(void* addr, size_t size);
    bool injected() const { return injected_; }
    dt::Address get_saved_reg(dt::MachRegister reg, dt::Address sp, size_t offset);
    static bool is_pc(dt::MachRegister);
    bool is_dyninst_lib(string lib);
    void set_jump_inst(bool b) { jump_ = b; }
    // only works for trap-based instrumentation
    void set_old_context(ucontext_t* c) { old_context_ = *c; }

    size_t sp_offset() const { return sp_offset_; }
    void set_sp_offset(size_t s) { sp_offset_ = s; }
    string exe_name() const { return exe_name_; }
  protected:
    typedef std::vector<pe::CodeSource*> CodeSources;
    CodeSources code_srcs_;
    CodeObjects code_objs_;
    ph::PatchObject* exe_obj_;
    ph::PatchMgrPtr mgr_;
    bool injected_;
    ucontext_t old_context_;
    std::vector<string> dyninst_libs_;
    bool jump_;
    size_t sp_offset_;  // offset of stack pointer for saved context

    typedef std::map<string,
      ph::PatchFunction*> RealFuncMap;
    RealFuncMap real_func_map_;

    string agent_name_;
    string exe_name_;

    SpParser();
    void init_dyninst_libs();
};

}

#endif /* SP_PARSER_H_ */
