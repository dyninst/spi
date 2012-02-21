#ifndef _SPSNIPPET_H_
#define _SPSNIPPET_H_

#include "SpCommon.h"
#include "SpPayload.h"

namespace sp {

	class SpFunction;
// ----------------------------------------------------------------------
// Snippet to hold relocated code and the invocation of payload function.
// ----------------------------------------------------------------------

	class SpSnippet {
		friend class SpInstrumenter;
		friend class RelocCallBlockWorker;
		friend class SpringboardWorker;
  public:
    typedef dyn_detail::boost::shared_ptr<SpSnippet> ptr;
    static ptr create(SpFunction* f,
                      SpPoint* pt,
                      PayloadFunc entry,
                      PayloadFunc exit) {
      return ptr(new SpSnippet(f, pt, entry, exit));
    }
    SpSnippet(SpFunction* f,
              SpPoint* pt,
              PayloadFunc entry,
							PayloadFunc exit);
    ~SpSnippet();

    // Return the pointer to blob, but blob is empty
		// If estimate_size is non-zero, we (re)allocate blob
    dt::Address get_blob(size_t estimate_size = 0);

    // Return the pointer to blob, and fill the blob
    char* build_blob(size_t est_size,
										 bool reloc = false);

    // Return the pointer to spring, and fill in spring
    char* spring(SpBlock* spring_blk);

    // blob size
    size_t size() const { return blob_size_; }

    // spring block size
    size_t spring_size() const {return spring_size_; }

    dt::Address get_saved_reg(dt::MachRegister reg);
    long get_ret_val();
    void* pop_argument(ArgumentHandle* h, size_t size);

    // Some getters
    PayloadFunc entry() const { return entry_; }
    PayloadFunc exit() const { return exit_; }
    SpPoint* point() const { return point_; }
    SpFunction* func() const { return func_; }

    // Find and return a spring block; if not found, return NULL
    SpBlock* spring_blk();

    static dt::Address get_pre_signal_pc(void* context);
    static dt::Address set_pc(dt::Address pc, void* context);
    static size_t jump_abs_size();

  protected:
    SpFunction* func_;
    SpPoint* point_;
    PayloadFunc entry_;
    PayloadFunc exit_;
    long saved_context_loc_;

    // Blob things
    char* blob_;
    size_t blob_size_;

    // Spring block things
    char* spring_;
    size_t spring_size_;
    SpBlock* spring_blk_;

    // A bunch of code generation interfaces
    size_t emit_save(char* buf, size_t offset);
    size_t emit_restore( char* buf, size_t offset);
    size_t emit_fault(char* buf, size_t offset);
    size_t emit_pass_param(long point, long payload, char* buf, size_t offset);
    size_t emit_call_abs(long callee, char* buf, size_t offset, bool restore);
    size_t emit_call_orig(char* buf, size_t offset);
    size_t emit_save_sp(char* buf, size_t offset);
    size_t emit_jump_abs(long trg, char* buf, size_t offset, bool abs = false);

    // relocate
    size_t reloc_block(SpBlock* blk,
                       char* buf,
                       size_t offset);
    size_t reloc_insn(dt::Address src_insn,
                      in::Instruction::Ptr insn,
                      dt::Address last,
                      char* buf);

	};

}

#endif /* _SPSNIPPET_H_ */
