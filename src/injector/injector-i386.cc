#include "SpInjector.h"
#include "SpCommon.h"

using sp::SpInjector;

// The code snippet to invoke do_dlopen
static char dlopen_code[] = {
  0x90, 0x90,                 // nop, nop
  0x68, 0x0, 0x0, 0x0, 0x0,   // pushl args
  0xe8, 0x0, 0x0, 0x0, 0x0,   // call do_dlopen
  0x83, 0xc4, 0x04,           // addl $04, %esp
  0xcc
};

static char ijagent_code[] = {
  0x90, 0x90,                 // nop, nop
  0xe8, 0x0, 0x0, 0x0, 0x0,   // call ij_agent
  0xcc
};

enum {
  OFF_ARGS = 3,
  OFF_DLOPEN = 8,
  OFF_DLRET = 12,

  OFF_IJ = 3,
  OFF_IJRET = 7
};

size_t SpInjector::get_code_tmpl_size() {
  return sizeof(dlopen_code);
}

char* SpInjector::get_code_tmpl(Dyninst::Address args_addr,
																Dyninst::Address do_dlopen,
																Dyninst::Address code_addr) {

  long* p = (long*)&dlopen_code[OFF_DLOPEN];
  Dyninst::Address abs_ret = code_addr + OFF_DLRET;
  *p = (long)do_dlopen - (long)abs_ret;
  p = (long*)&dlopen_code[OFF_ARGS];
  *p = (long)args_addr;

  return dlopen_code;
}

size_t SpInjector::get_ij_tmpl_size() {
  return sizeof(ijagent_code);
}

char* SpInjector::get_ij_tmpl(Dyninst::Address ij_addr,
															Dyninst::Address code_addr) {
  Dyninst::Address abs_ret = code_addr + OFF_IJRET;
  long* p = (long*)&ijagent_code[OFF_IJ];
  *p = (long)ij_addr - (long)abs_ret;
  return ijagent_code;
}

Dyninst::Address SpInjector::get_pc() {
  Dyninst::MachRegisterVal eip;
  thr_->getRegister(Dyninst::x86::eip, eip);
  return eip;
}

Dyninst::Address SpInjector::get_sp() {
  Dyninst::MachRegisterVal esp;
  thr_->getRegister(Dyninst::x86::esp, esp);
  return esp;
}

Dyninst::Address SpInjector::get_bp() {
  Dyninst::MachRegisterVal ebp;
  thr_->getRegister(Dyninst::x86::ebp, ebp);
  return ebp;
}
