#ifndef _SPSNIPPET_H_
#define _SPSNIPPET_H_

#include <sys/ucontext.h>

#include "SpCommon.h"
#include "SpPayload.h"

namespace sp {

class SpSnippet {
  public:
    typedef dyn_detail::boost::shared_ptr<SpSnippet> ptr;
    static ptr create(Dyninst::PatchAPI::PatchFunction* f,
                      Dyninst::PatchAPI::Point* pt,
                      SpContext* c,
                      PayloadFunc p) {
      return ptr(new SpSnippet(f, pt, c, p));
    }

    SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
              Dyninst::PatchAPI::Point* pt,
              SpContext* c,
              PayloadFunc p);
    ~SpSnippet();

    char* blob(Dyninst::Address ret_addr);
    size_t size() { return blob_size_; }

    SpContext* context() { return context_; }
    PayloadFunc payload() { return payload_; }
    string& orig_insn() { return orig_insn_; }
    Dyninst::PatchAPI::PatchFunction* func() { return func_; }

    static void dump_context(ucontext_t* context);
    static Dyninst::Address get_pre_signal_pc(void* context);
    static Dyninst::Address set_pc(Dyninst::Address pc, void* context);
  protected:
    Dyninst::PatchAPI::PatchFunction* func_;
    Dyninst::PatchAPI::Point* point_;
    SpContext* context_;
    PayloadFunc payload_;
    string orig_insn_;
    char* blob_;
    size_t blob_size_;

    // A bunch of code generation interfaces
    static size_t emit_save(char* buf, size_t offset);
    static size_t emit_restore( char* buf, size_t offset);
    static size_t emit_fault(char* buf, size_t offset);
    static size_t emit_pass_param(long point, long context,
                                  char* buf, size_t offset);
    static size_t emit_call_abs(long callee, char* buf, size_t offset);
    static size_t emit_call_jump(long callee, char* buf, size_t offset);
    static size_t emit_jump_abs(long trg, char* buf, size_t offset);
};

}

#endif /* _SPSNIPPET_H_ */
