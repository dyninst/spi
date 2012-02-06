#include "SpEvent.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpSnippet.h"

namespace sp {
	extern SpContext* g_context;

  // -------------------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------------------

  // Save context before calling payload
  size_t
  SpSnippet::emit_save(char* buf,
											 size_t offset) {
    char* p = buf + offset;
    // pusha
    *p++ = 0x60;
    size_t insnsize = emit_save_sp(p, 0);
    p += insnsize;
    return (p - (buf + offset));
  }

  // Restore context after calling payload
  size_t
  SpSnippet::emit_restore(char* buf,
													size_t offset) {
    char* p = buf + offset;
    // popa
    *p++ = 0x61;
    return (p - (buf + offset));
  }

  // For debugging, cause segment fault
  size_t
  SpSnippet::emit_fault(char* buf,
												size_t offset) {
    char* p = buf + offset;

    // mov $0, 0
    *p++ = (char)0xc7;
    *p++ = (char)0x05;

    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;

    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;
    *p++ = (char)0x00;

    return (p - (buf + offset));
  }

  // Save stack pionter, for two purposes
  // 1. Resolve indirect call during runtime
  // 2. Get argument of callees in payload function
  size_t
  SpSnippet::emit_save_sp(char* buf,
													size_t offset) {
    char* p = buf + offset;

    *p++ = 0x89;   // mov %esp, (loc)
    *p++ = 0x25;

    long* l = (long*)p;
    *l = (long)&saved_context_loc_;
    p += sizeof(long);

    return (p - (buf + offset));
  }

  // Push an imm32 in stack
  static size_t
  emit_push_imm32(long imm,
									char* buf,
									size_t offset) {
    char* p = buf + offset;

    // push imm32
    *p++ = 0x68;
    long* i = (long*)p;
    *i = imm;
    p += sizeof(long);

    return (p - (buf + offset));
  }

  // Get the saved imm32 from stack to %eax
  static size_t
  emit_pop_imm32(char* buf,
								 size_t offset) {
    char* p = buf + offset;
    // pop %eax
    *p++ = 0x58;
    return (p - (buf + offset));
  }

  // Pass parameter to payload function
  // Two parameters - POINT and Payload function
  // If payload == 0, then we are dealing with single-process only
  size_t
  SpSnippet::emit_pass_param(long point,
														 long payload,
														 char* buf,
														 size_t offset) {
    char* p = buf + offset;
    size_t insnsize = 0;

    if (payload) {
      // push payload
      insnsize = emit_push_imm32((long)payload, p, 0);
      p += insnsize;
    }

    // push POINT
    insnsize = emit_push_imm32((long)point, p, 0);
    p += insnsize;

    return (p - (buf + offset));
  }

  // Call a function w/ address `callee`
  // `restore` indicates whether we need to pop out the pushed argument,
  // this only happens when we call payload function
  size_t
  SpSnippet::emit_call_abs(long callee,
													 char* buf,
                           size_t offset,
													 bool restore) {
    char* p = buf + offset;
    dt::Address retaddr = (dt::Address)p+5;
    size_t insnsize = 0;
    dt::Address rel_addr = (callee - retaddr);

    if (sp::is_disp32(rel_addr)) {
      // call callee
      *p++ = 0xe8;
      int* rel_p = (int*)p;
      *rel_p = rel_addr;
      p += sizeof(long);
    } else {
      sp_perror("larger than 4 bytes for call address");
    }

    if (restore) {
      // pop point
      insnsize = emit_pop_imm32(p, 0);
      p += insnsize;

      if (g_context->allow_ipc()) {
        // pop payload
        insnsize = emit_pop_imm32(p, 0);
        p += insnsize;
      }
    }

    return (p - (buf + offset));
  }

  // Jump to `trg`.
  // `abs` indicates whether we must use absolute jump. If `abs` is false,
  // we make such decision dynamically; otherwise, we mandate absolute
  // jump for determinism.
  size_t
  SpSnippet::emit_jump_abs(long trg,
													 char* buf,
													 size_t offset,
													 bool abs) {
    char* p = buf + offset;
    size_t insnsize = 0;

    dt::Address retaddr = (dt::Address)p+5;
    dt::Address rel_addr = (trg - retaddr);

    if (sp::is_disp32(rel_addr) && !abs) {
      // jmp trg
      *p++ = 0xe9;
      int* rel_p = (int*)p;
      *rel_p = rel_addr;
      p += 4;
    } else {
      // push trg
      insnsize = emit_push_imm32(trg, p, 0);
      p += insnsize;
      // ret
      *p++ = 0xc3;
    }

    return (p - (buf + offset));
  }

  // Relocate the call instruction
  // This is used in dealing with indirect call
  size_t
  SpSnippet::emit_call_orig(char* buf,
														size_t offset) {
		SpBlock* b = point_->get_block();
		assert(b);

    char* p = buf + offset;
    char* psrc = (char*)b->orig_call_insn()->ptr();
		size_t size = b->orig_call_insn()->size();
    memcpy(p, psrc, size);
    return size;
  }

  // Relocate an ordinary instruction
  size_t
  SpSnippet::reloc_insn(dt::Address src_insn,
												in::Instruction::Ptr insn,
                        dt::Address last,
												char* buf) {
    // Case 1: we don't relocate the last insn
    // because we'll do it later
    dt::Address a = src_insn;
    if (a == last)  return 0;

    char* p = buf;
    if (insn->getCategory() == in::c_CallInsn &&
        a != last)  {
			sp_debug("THUNK CALL - at insn %lx", src_insn);
      // Case 2: handle thunk call
      // What thunk does, is to move current pc value to ebx.
      // mov orig_pc, ebx
      *p++ = 0xbb;
      long* new_ebx = (long*)p;
      *new_ebx = (long)(a+5);
      return 5;
    } else {
      // Case 3: other instructions
      memcpy(p, insn->ptr(), insn->size());
      return insn->size();
    }
  }

  // -------------------------------------------------------------------
  // Miscellaneous
  // -------------------------------------------------------------------

  // Used in trap handler to decide the pc value right at the call
  dt::Address
  SpSnippet::get_pre_signal_pc(void* context) {
    ucontext_t* ctx = (ucontext_t*)context;
    return ctx->uc_mcontext.gregs[REG_EIP];
  }

  // Used in trap handler to jump to snippet
  dt::Address
  SpSnippet::set_pc(dt::Address pc,
										void* context) {
    ucontext_t* ctx = (ucontext_t*)context;
    ctx->uc_mcontext.gregs[REG_EIP] = pc;
    return pc;
  }

  // Get the saved register, for resolving indirect call
  dt::Address
  SpSnippet::get_saved_reg(Dyninst::MachRegister reg) {

    // Pushed Left to right in order:
    // Push(EAX); Push(ECX); Push(EDX); Push(EBX); Push(Temp); Push(EBP); Push(ESI); Push(EDI);
#define EDI  0
#define ESI  4
#define EBP  8
#define ESP  12
#define EBX  16
#define EDX  20
#define ECX  24
#define EAX  28

#define reg_val(i) (*(long*)(saved_context_loc_+(i)))
    /*
      for (int i = 0; i < 32; i+=4) {
      sp_debug("i: %d, EDI: %lx", i, reg_val(i));
      }
    */
    using namespace Dyninst::x86;
    if (reg == edi) return reg_val(EDI);
    if (reg == esi) return reg_val(ESI);
    if (reg == ebp) return reg_val(EBP);
    if (reg == esp) return reg_val(ESP);
    if (reg == ebx) return reg_val(EBX);
    if (reg == edx) return reg_val(EDX);
    if (reg == ecx) return reg_val(ECX);
    if (reg == eax) return reg_val(EAX);

    return 0;
  }

  // Is this register EIP?
  bool
  is_pc(Dyninst::MachRegister r) {
    if (r == Dyninst::x86::eip) return true;
    return false;
  }


  // The upper bound for a jump instruction.
  size_t
  SpSnippet::jump_abs_size() {
    // pushl imm;
    // ret
    return 7;
  }

  // Get argument of a function call
  void*
  SpSnippet::pop_argument(ArgumentHandle* h,
													size_t size) {
    void* a = (void*)(saved_context_loc_ + 32 + h->offset);
    h->offset += size;
    return a;
  }

  // Get return value of a function call
  long
  SpSnippet::get_ret_val() {
    return get_saved_reg(Dyninst::x86::eax);
  }

}
