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

// This enables to inject a shared library into a running
// process's address space.

#ifndef SP_INJECTOR_H_
#define SP_INJECTOR_H_

// ProcControlAPI
#include "PCProcess.h"

// SelfPropelled
#include "common/common.h"
#include "common/utils.h"

namespace sp {

  class SpInjector {
 public:
    typedef boost::shared_ptr<SpInjector> ptr;

    INJECTOR_EXPORT static ptr Create(dt::PID pid);
    INJECTOR_EXPORT bool Inject(const char* lib_name);

    ~SpInjector();
 protected:
    // The argument type, passed to do_dlopen
    typedef struct {
      const char *libname;
      int mode;
      void* link_map;
    } DlopenArg;


    dt::PID pid_;
    dt::ProcControlAPI::Process::ptr proc_;
    dt::ProcControlAPI::Thread::ptr thr_;
    StringSet dep_names_;

    SpInjector(dt::PID pid);

    // Find a function from all loaded libraries and executable
    dt::Address FindFunction(const char* name);

    // Find dlopen function
    dt::Address FindDlopen();

    // Get full paths for a given library name
    bool GetResolvedLibPath(const std::string &filename,
                            StringSet &paths);

    // The first step: load ijagent.so into mutatee
    void LoadIjagent(const char* libname);

    // The second step: load user library
    void LoadUserLibrary();

    void UpdateFrameInfo();

    bool IsLibraryLoaded(const char* libname);

    // Following methods are platform-dependent methods.
    // See Injector-i386.C and Injector-x86_64

    // Get three registers to assist getting stack frames
    dt::Address pc();
    dt::Address sp();
    dt::Address bp();

    // For loading user library
    size_t GetIjTemplateSize();
    unsigned char* GetIjTemplate(dt::Address ij_addr,
                        dt::Address code_addr);

    // For loading ijagent.so using do_dlopen
    size_t GetCodeTemplateSize();
    unsigned char* GetCodeTemplate(dt::Address args_addr,
                          dt::Address do_dlopen,
                          dt::Address code_addr);

    // For loading ijagent.so using __libc_dlopen_mode
    size_t GetDlmodeTemplateSize();
    unsigned char* GetDlmodeTemplate(Dyninst::Address libname,
                            Dyninst::Address mode,
                            Dyninst::Address dlopen,
                            Dyninst::Address /*code_addr*/);
    
  };

}

#endif /* SP_INJECTOR_H_ */
