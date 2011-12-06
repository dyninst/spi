#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "SpAgentCommon.h"
#include "SpSnippet.h"

namespace sp {
class SpInstrumenter : public ph::Instrumenter {
  public:
    static SpInstrumenter* create(ph::AddrSpace* as);
    virtual bool run();

  protected:
    SpInstrumenter(ph::AddrSpace*);

    bool install_direct(ph::Point* point,
                        char*      blob,
                        size_t     blob_size);

    bool install_indirect(ph::Point*         point, 
                          sp::SpSnippet::ptr snip,
                          bool               jump_abs,
                          dt::Address        ret_addr);

    bool install_jump(ph::PatchBlock*     blk,
                      char*               insn,
                      size_t              insn_size,
                      sp::SpSnippet::ptr  snip, 
                      dt::Address         ret_addr);

    bool install_spring(ph::PatchBlock*    callblk,
                        sp::SpSnippet::ptr snip,
                        dt::Address        ret_addr);

    bool install_trap(ph::Point* point,
                      char*      blob,
                      size_t     blob_size);

    static void trap_handler(int        sig,
                             siginfo_t* info,
                             void*      c);
};

}

#endif /* _SPINSTRUMENTER_H_ */
