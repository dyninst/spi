#include "Injector.h"
#include "Common.h"

using sp::Injector;

/* The code snippet to invoke do_dlopen */
static char dlopen_code[] = {
  0x90, 0x90,                 // nop, nop
  0x68, 0x0, 0x0, 0x0, 0x0,   // pushl args
  0xe8, 0x0, 0x0, 0x0, 0x0,   // call do_dlopen
  0x83, 0xc4, 0x04,           // addl $04, %esp
  0xcc
};

enum {
  OFF_ARGS = 3,
  OFF_DLOPEN = 8,
  OFF_DLRET = 12
};

size_t Injector::get_code_tmpl_size() {
  return sizeof(dlopen_code);
}

char* Injector::get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                              Dyninst::Address code_addr) {

  long* p = (long*)&dlopen_code[OFF_DLOPEN];
  Dyninst::Address abs_ret = code_addr + OFF_DLRET;
  *p = (long)do_dlopen - (long)abs_ret;
  p = (long*)&dlopen_code[OFF_ARGS];
  *p = (long)args_addr;

  return dlopen_code;
}

