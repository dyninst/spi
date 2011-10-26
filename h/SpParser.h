#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "PatchObject.h"
#include "PatchMgr.h"
#include <ucontext.h>
#include <ext/hash_map>

namespace sp {

/*
   Parser is to parse the CFG structures of the mutatee process.
 */
class SpParser : public Dyninst::PatchAPI::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<SpParser> ptr;
    virtual ~SpParser();
    static ptr create();

    typedef std::vector<Dyninst::ParseAPI::CodeObject*> CodeObjects;
    typedef std::vector<Dyninst::PatchAPI::PatchObject*> PatchObjects;
    virtual Dyninst::PatchAPI::PatchMgrPtr parse();
    Dyninst::PatchAPI::PatchObject* exe_obj();
    Dyninst::PatchAPI::PatchFunction* findFunction(Dyninst::Address addr);
    Dyninst::PatchAPI::PatchFunction* findFunction(string name, bool skip = true);
    Dyninst::PatchAPI::PatchFunction* callee(Dyninst::PatchAPI::Point* pt,
                                             bool parse_indirect = false);

    char* get_agent_name();
    Dyninst::Address get_func_addr(string name);
    string dump_insn(void* addr, size_t size);
    bool injected() const { return injected_; }
    Dyninst::Address get_saved_reg(Dyninst::MachRegister reg, Dyninst::Address sp, size_t orig_insn_size);
    static bool is_pc(Dyninst::MachRegister);
    bool is_dyninst_lib(string lib);
    void set_jump_inst(bool b) { jump_ = b; }
    // only works for trap-based instrumentation
    void set_old_context(ucontext_t* c) { old_context_ = *c; }
  protected:
    typedef std::vector<Dyninst::ParseAPI::CodeSource*> CodeSources;
    CodeSources code_srcs_;
    CodeObjects code_objs_;
    Dyninst::PatchAPI::PatchObject* exe_obj_;
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    bool injected_;
    ucontext_t old_context_;
    std::vector<string> dyninst_libs_;
    bool jump_;

    typedef std::map<string,
      Dyninst::PatchAPI::PatchFunction*> RealFuncMap;
    RealFuncMap real_func_map_;

    SpParser();
    void init_dyninst_libs();
};

}

#endif /* SP_PARSER_H_ */
