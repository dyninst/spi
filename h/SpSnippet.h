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
    void set_old_context(ucontext_t* old_context) { old_context_ = old_context; }
    string& orig_insn() { return orig_insn_; }
    Dyninst::PatchAPI::PatchFunction* func() { return func_; }
  protected:
    Dyninst::PatchAPI::PatchFunction* func_;
    Dyninst::PatchAPI::Point* point_;
    SpContext* context_;
    PayloadFunc payload_;
    string orig_insn_;
    Dyninst::Address setcontext_func_;
    Dyninst::Address getcontext_func_;

    char* blob_;
    size_t blob_size_;
    ucontext_t* old_context_;
};

}

#endif /* _SPSNIPPET_H_ */
