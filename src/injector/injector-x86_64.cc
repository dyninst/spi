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

static char kIjagentCode[] = {
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

size_t
SpInjector::GetCodeTemplateSize() {
  return sizeof(kDlopenCode);
}

char*
SpInjector::GetCodeTemplate(Dyninst::Address args_addr,
                            Dyninst::Address do_dlopen,
                            Dyninst::Address /*code_addr*/) {
  long* p = (long*)&kDlopenCode[OFF_DODLOPEN];
  *p = (long)do_dlopen;
  p = (long*)&kDlopenCode[OFF_ARGS];
  *p = (long)args_addr;
  return kDlopenCode;
}

size_t SpInjector::GetIjTemplateSize() {
  return sizeof(kIjagentCode);
}

char* SpInjector::GetIjTemplate(Dyninst::Address ij_addr,
                                Dyninst::Address /*code_addr*/) {
  long* p = (long*)&kIjagentCode[OFF_IJ];
  *p = (long)ij_addr;
  return kIjagentCode;
}

Dyninst::Address SpInjector::pc() {
  Dyninst::MachRegisterVal rip;
  thr_->getRegister(Dyninst::x86_64::rip, rip);
  return rip;
}

Dyninst::Address SpInjector::sp() {
  Dyninst::MachRegisterVal rsp;
  thr_->getRegister(Dyninst::x86_64::rsp, rsp);
  return rsp;
}

Dyninst::Address SpInjector::bp() {
  Dyninst::MachRegisterVal rbp;
  thr_->getRegister(Dyninst::x86_64::rbp, rbp);
  return rbp;
}

}
