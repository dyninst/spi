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
                      PayloadFunc head, PayloadFunc tail) {
      return ptr(new SpSnippet(f, pt, c, head, tail));
    }

    SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
              Dyninst::PatchAPI::Point* pt,
              SpContext* c,
              PayloadFunc head, PayloadFunc tail);
    ~SpSnippet();

    char* blob(Dyninst::Address ret_addr, bool reloc = false,  bool spring = false);
    char* spring(Dyninst::Address ret_addr);
    void fixup(Dyninst::PatchAPI::PatchFunction* f);  // for call imm(rip)

    size_t size() { return blob_size_; }
    size_t spring_size() {return spring_size_; }
    SpContext* context() { return context_; }
    PayloadFunc head() { return head_; }
    PayloadFunc tail() { return tail_; }

    string& orig_insn() { return orig_insn_; }
    string& orig_blk() { return orig_blk_; }
    string& orig_spring_blk() { return orig_spring_blk_; }
    Dyninst::PatchAPI::PatchBlock* spring_blk();

    Dyninst::PatchAPI::PatchFunction* func() { return func_; }
    Dyninst::Address buf() const { return (Dyninst::Address)blob_; }

    static void dump_context(ucontext_t* context);
    static Dyninst::Address get_pre_signal_pc(void* context);
    static Dyninst::Address set_pc(Dyninst::Address pc, void* context);
    static size_t jump_abs_size();
    static size_t emit_jump_abs(long trg, char* buf, size_t offset, bool abs = false);

  protected:
    Dyninst::PatchAPI::PatchFunction* func_;
    Dyninst::PatchAPI::Point* point_;
    SpContext* context_;
    PayloadFunc head_;
    PayloadFunc tail_;
    string orig_insn_;
    string orig_blk_;
    string orig_spring_blk_;
    char* blob_;
    size_t blob_size_;
    size_t before_call_orig_;
    Dyninst::Address ret_addr_;
    char* spring_;
    size_t spring_size_;
    Dyninst::PatchAPI::PatchBlock* spring_blk_;

    // A bunch of code generation interfaces
    static size_t emit_save(char* buf, size_t offset);
    static size_t emit_restore( char* buf, size_t offset);
    static size_t emit_fault(char* buf, size_t offset);
    static size_t emit_pass_param(long point, char* buf, size_t offset);
    static size_t emit_call_abs(long callee, char* buf, size_t offset, bool restore);
    static size_t emit_ret(char* buf, size_t offset);
    static size_t emit_call_jump(long callee, char* buf, size_t offset);
    static size_t emit_call_orig(long src, size_t size, char* buf, size_t offset);
    static size_t emit_save_sp(long loc, char* buf, size_t offset);

    // relocate
    size_t reloc_block(Dyninst::PatchAPI::PatchBlock* blk, char* buf, size_t offset);
    size_t reloc_insn(Dyninst::PatchAPI::PatchBlock::Insns::iterator i,
                      Dyninst::Address last, char* p);
};

}

#endif /* _SPSNIPPET_H_ */
