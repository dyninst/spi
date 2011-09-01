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
                      SpContextPtr c,
                      PayloadFunc p) {
      return ptr(new SpSnippet(f, pt, c, p));
    }

    SpSnippet(Dyninst::PatchAPI::PatchFunction* f,
              Dyninst::PatchAPI::Point* pt,
              SpContextPtr c,
              PayloadFunc p)
              : func_(f), point_(pt), context_(c), payload_(p), old_context_(NULL) {
    }
    ~SpSnippet();

    char* blob();
    SpContextPtr context() { return context_; }
    PayloadFunc payload() { return payload_; }
    string& orig_insn() { return orig_insn_; }
    Dyninst::PatchAPI::PatchFunction* func() { return func_; }
  protected:
    Dyninst::PatchAPI::PatchFunction* func_;
    Dyninst::PatchAPI::Point* point_;
    SpContextPtr context_;
    PayloadFunc payload_;
    string orig_insn_;

    string blob_;
    ucontext_t* old_context_;
};

}

#endif /* _SPSNIPPET_H_ */
