#include "Injector.h"

using sp::Injector;

/* The code snippet to invoke do_dlopen */
static char dlopen_code[] = {
  0x48, 0xc7, 0xc0, 0x0, 0x0, 0x0, 0x0, // movq args, %rax
  0x50,                                 // pushl %rax
  0xe8, 0x0, 0x0, 0x0, 0x0,             // call do_dlopen
  0x58,
  0xCC
};

enum {
  OFF_ARGS = 3,
  OFF_DLOPEN = 9,
  OFF_DLRET = 13
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
