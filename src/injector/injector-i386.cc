/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "injector/injector.h"

namespace sp {
// The code snippet to invoke do_dlopen
static char kDlopenCode[] = {
  0x90, 0x90,                 // nop, nop
  0x68, 0x0, 0x0, 0x0, 0x0,   // pushl args
  0xe8, 0x0, 0x0, 0x0, 0x0,   // call do_dlopen
  0x83, 0xc4, 0x04,           // addl $04, %esp
  0xcc
};

// The code snippet to invoke __libc_dlopen_mode
static char kLibcDlopenModeCode[] = {
  // 0, 1
  0x90, 0x90,                 // nop, nop
  // 2,   3,   4,   5,   6,
  //    OFF_ARGS  
  0x68, 0x0, 0x0, 0x0, 0x0,   // pushl mode
  // 7,   8,   9,  10,  11,
  //    OFF_ARGS+5  
  0x68, 0x0, 0x0, 0x0, 0x0,   // pushl libname
  //12,  13,  14,  15,  16,
  //    OFF_DLOPEN+5  
  0xe8, 0x0, 0x0, 0x0, 0x0,   // call __libc_dlopen_mode
  //17,   18,   19,
  // OFF_RET + 5
  0x83, 0xc4, 0x08,           // addl $08, %esp
  //20
  0xcc
};

static char kIjagentCode[] = {
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

size_t SpInjector::GetCodeTemplateSize() {
  return sizeof(kDlopenCode);
}

char* SpInjector::GetCodeTemplate(Dyninst::Address args_addr,
                                  Dyninst::Address do_dlopen,
                                  Dyninst::Address code_addr) {

  long* p = (long*)&kDlopenCode[OFF_DLOPEN];
  Dyninst::Address abs_ret = code_addr + OFF_DLRET;
  *p = (long)do_dlopen - (long)abs_ret;
  p = (long*)&kDlopenCode[OFF_ARGS];
  *p = (long)args_addr;

  return kDlopenCode;
}

size_t SpInjector::GetIjTemplateSize() {
  return sizeof(kIjagentCode);
}

char* SpInjector::GetIjTemplate(Dyninst::Address ij_addr,
                                Dyninst::Address code_addr) {
  Dyninst::Address abs_ret = code_addr + OFF_IJRET;
  long* p = (long*)&kIjagentCode[OFF_IJ];
  *p = (long)ij_addr - (long)abs_ret;
  return kIjagentCode;
}

size_t
SpInjector::GetDlmodeTemplateSize() {
  return sizeof(kLibcDlopenModeCode);
}

char*
SpInjector::GetDlmodeTemplate(Dyninst::Address libname,
                              Dyninst::Address mode,
                              Dyninst::Address dlopen,
                              Dyninst::Address code_addr) {
  Dyninst::Address abs_ret = code_addr + OFF_DLRET + 5;
  long* p = (long*)&kLibcDlopenModeCode[OFF_DLOPEN+5];
  *p = (long)dlopen - (long)abs_ret;

  p = (long*)&kLibcDlopenModeCode[OFF_ARGS];
  *p = (int)mode;

  p = (long*)&kLibcDlopenModeCode[OFF_ARGS+5];
  *p = (long)libname;


  return kLibcDlopenModeCode;
}

Dyninst::Address SpInjector::pc() {
  Dyninst::MachRegisterVal eip;
  thr_->getRegister(Dyninst::x86::eip, eip);
  return eip;
}

Dyninst::Address SpInjector::sp() {
  Dyninst::MachRegisterVal esp;
  thr_->getRegister(Dyninst::x86::esp, esp);
  return esp;
}

Dyninst::Address SpInjector::bp() {
  Dyninst::MachRegisterVal ebp;
  thr_->getRegister(Dyninst::x86::ebp, ebp);
  return ebp;
}

}
