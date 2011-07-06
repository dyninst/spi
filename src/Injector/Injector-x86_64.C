#include "Injector.h"

using sp::Injector;

/* The code snippet to invoke do_dlopen */
static char dlopen_code[] = {
  //0 , 1
  0x90, 0x90,                                         // nop, nop
  //         (OFF_ARGS)
  //2 , 3,    4,    5,   6,   7,   8,   9,   10,  11
  0x48, 0xbf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // movq args, %rdi
  //         (OFF_DLOPEN)
  //12, 13,   14,  15,  16,  17,  18,  19,  20,  21
  0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // movq do_dlopen, %rax
  //(OFF_DLRET)
  //22, 23
  0xff, 0xd0,                                         // call %rax
  0x58,                                               // pop eax
  0xCC
};

enum {
  OFF_ARGS = 4,
  OFF_DLOPEN = 14,
  OFF_DLRET = 22
};

size_t Injector::get_code_tmpl_size() {
  return sizeof(dlopen_code);
}

char* Injector::get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                              Dyninst::Address /*code_addr*/) {
  long* p = (long*)&dlopen_code[OFF_DLOPEN];
  *p = (long)do_dlopen;
  p = (long*)&dlopen_code[OFF_ARGS];
  *p = (long)args_addr;
  return dlopen_code;
}
