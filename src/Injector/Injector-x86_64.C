#include "Injector.h"
#include <dlfcn.h>

using sp::Injector;

/* The code snippet to invoke do_dlopen */
static char do_dlopen_code[] = {
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
  0xCC
};

static char ijagent_code[] = {
  //0 , 1
  0x90, 0x90,                                         // nop, nop
  //         (OFF_IJ)
  //2 , 3,    4,    5,   6,   7,   8,   9,   10,  11
  0x48, 0xb8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // movq ij_agent, %rax
  //(OFF_IJRET)
  //12, 13
  0xff, 0xd0,                                         // call %rax
  0xCC
};

enum {
  OFF_ARGS = 4,
  OFF_DODLOPEN = 14,
  OFF_DODLRET = 22,

  OFF_IJ = 4,
  OFF_IJRET = 12
};

size_t Injector::get_code_tmpl_size() {
  return sizeof(do_dlopen_code);
}

char* Injector::get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                              Dyninst::Address /*code_addr*/) {
  long* p = (long*)&do_dlopen_code[OFF_DODLOPEN];
  *p = (long)do_dlopen;
  p = (long*)&do_dlopen_code[OFF_ARGS];
  *p = (long)args_addr;
  return do_dlopen_code;
}

size_t Injector::get_ij_tmpl_size() {
  return sizeof(ijagent_code);
}

char* Injector::get_ij_tmpl(Dyninst::Address ij_addr,
                            Dyninst::Address /*code_addr*/) {
  long* p = (long*)&ijagent_code[OFF_IJ];
  *p = (long)ij_addr;
  return ijagent_code;
}
