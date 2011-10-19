#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "Instrumenter.h"
#include "SpCommon.h"
#include "SpSnippet.h"

namespace sp {
class JumpInstrumenter : public Dyninst::PatchAPI::Instrumenter {
  public:
    static JumpInstrumenter* create(Dyninst::PatchAPI::AddrSpace* as);
    virtual bool run();

  protected:
    JumpInstrumenter(Dyninst::PatchAPI::AddrSpace*);
    bool install_direct(Dyninst::PatchAPI::Point* point, char* blob, size_t blob_size);
    bool install_indirect(Dyninst::PatchAPI::Point* point, sp::SpSnippet::ptr snip,
                          bool jump_abs, Dyninst::Address ret_addr);

    bool install_jump(Dyninst::PatchAPI::PatchBlock* blk, char* insn, size_t insn_size,
                      sp::SpSnippet::ptr snip, Dyninst::Address ret_addr);
    bool install_spring(Dyninst::PatchAPI::PatchBlock* callblk, sp::SpSnippet::ptr snip,
                        Dyninst::Address ret_addr);
};

}

#endif /* _SPINSTRUMENTER_H_ */
