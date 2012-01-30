#ifndef _SPSNIPPET_H_
#define _SPSNIPPET_H_

#include "SpAgentCommon.h"
#include "SpPayload.h"

namespace sp {

// ----------------------------------------------------------------------------- 
// Snippet to hold relocated code and the invocation of payload function.
// -----------------------------------------------------------------------------

	class SpSnippet {
		friend class SpInstrumenter;
		friend class RelocCallBlockWorker;
		friend class SpringboardWorker;
  public:
    typedef dyn_detail::boost::shared_ptr<SpSnippet> ptr;
    static ptr create(ph::PatchFunction* f,
                      SpPoint* pt,
                      SpContext* c,
                      PayloadFunc entry,
                      PayloadFunc exit) {
      return ptr(new SpSnippet(f, pt, c, entry, exit));
    }
    SpSnippet(ph::PatchFunction* f,
              SpPoint* pt,
              SpContext* c,
              PayloadFunc entry, PayloadFunc exit);
    ~SpSnippet();

    // Return the pointer to blob, but blob is empty
    dt::Address buf() const { return (dt::Address)blob_; }
    char* realloc(); 

    // Return the pointer to blob, and fill the blob
    char* blob(bool reloc = false,  bool spring = false);

    // Return the pointer to spring, and fill in spring
    char* spring(dt::Address ret_addr);

    // blob size
    size_t size() const { return blob_size_; }

    // spring block size
    size_t spring_size() const {return spring_size_; }

    dt::Address get_saved_reg(dt::MachRegister reg);
    long get_ret_val();
    void* pop_argument(ArgumentHandle* h, size_t size);

    // Some getters
    SpContext* context() const { return context_; }
    PayloadFunc entry() const { return entry_; }
    PayloadFunc exit() const { return exit_; }
    SpPoint* point() const { return point_; }
    ph::PatchFunction* func() const { return func_; }

    string& orig_blk() { return orig_blk_; }
    string& orig_spring_blk() { return orig_spring_blk_; }

    // Find and return a spring block; if not found, return NULL
    ph::PatchBlock* spring_blk();


    static dt::Address get_pre_signal_pc(void* context);
    static dt::Address set_pc(dt::Address pc, void* context);
    static size_t jump_abs_size();

  protected:
    ph::PatchFunction* func_;
    SpPoint* point_;
    SpContext* context_;
    PayloadFunc entry_;
    PayloadFunc exit_;
    long saved_context_loc_;

    // Blob things
    char* blob_;
    size_t blob_size_;

    // Spring block things
    char* spring_;
    size_t spring_size_;
    string orig_blk_;
    string orig_spring_blk_;
    ph::PatchBlock* spring_blk_;

    dt::Address reloc_call_addr_;
    bool realloc_;

    // A bunch of code generation interfaces
    size_t emit_save(char* buf, size_t offset, bool indirect=false);
    size_t emit_restore( char* buf, size_t offset, bool indirect=false);
    size_t emit_fault(char* buf, size_t offset);
    size_t emit_pass_param(long point, long payload, char* buf, size_t offset);
    size_t emit_call_abs(long callee, char* buf, size_t offset, bool restore);
    size_t emit_call_orig(long src, size_t size, char* buf, size_t offset);
    size_t emit_save_sp(char* buf, size_t offset);
    size_t emit_jump_abs(long trg, char* buf, size_t offset, bool abs = false);

    // relocate
    size_t reloc_block(ph::PatchBlock* blk,
                       char* buf,
                       size_t offset);
    size_t reloc_insn(dt::Address src_insn,
                      in::Instruction::Ptr insn,
                      dt::Address last,
                      char* buf);

	};

}

#endif /* _SPSNIPPET_H_ */
