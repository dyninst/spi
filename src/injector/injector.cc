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

#include <sys/stat.h>
#include <dlfcn.h>
#include <signal.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Function.h"
#include "symtabAPI/h/AddrLookup.h"

#include "proccontrol/src/int_process.h"
#include "common/common.h"
#include "injector/injector.h"

#include "common/utils.h"

#define IJ_TIMEOUT 800

namespace sp {

SpInjector::ptr
SpInjector::Create(dt::PID pid) {
  SpInjector::ptr ret = ptr(new SpInjector(pid));
  return ret;
}

SpInjector::SpInjector(dt::PID pid)
    : pid_(pid) {

  proc_ = Process::attachProcess(pid);
  if (!proc_) {
    sp_perror("Injector [pid = %5d] - Failed to attach to process %d.",
              getpid(), pid);
  }
  ThreadPool& thrs = proc_->threads();
  thr_ = thrs.getInitialThread();
}

SpInjector::~SpInjector() {
  proc_->detach();
}

// Find do_dlopen
Address
SpInjector::FindFunction(const char* func) {
  LibraryPool& libs = proc_->libraries();

  for (LibraryPool::iterator li = libs.begin(); li != libs.end(); li++) {
    std::string name = (*li)->getName();
    if (name.size() <= 0) continue;
    sp_debug("IN LIB - %s", name.c_str());
    sb::Symtab *obj = NULL;
    bool ret = sb::Symtab::openFile(obj, name);
    std::vector <sb::Function *> funcs;
    if (!obj || !ret) {
      sp_perror("Injector [pid = %5d] - Failed to open %s",
                getpid(), sp_filename(name.c_str()));
    } else {
      obj->findFunctionsByName(funcs, func);
    }
    if (funcs.size() > 0) {
      return funcs[0]->getOffset() + (*li)->getLoadAddress();
    }
  }
  return 0;
}


// Event handlers

// For loading library event
Process::cb_ret_t
on_event_lib(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++) {
    Address loaded_addr = (*i)->getLoadAddress();
    sp_debug("LOADED - Library %s at %lx",
             sp_filename((*i)->getName().c_str()), loaded_addr);
  }
  return Process::cbThreadContinue;
}

// For segment fault event
Process::cb_ret_t
on_event_signal(Event::const_ptr ev) {
  EventSignal::const_ptr sigev = ev->getEventSignal();
  if(sigev->getSignal() == SIGSEGV) {
    sp_perror("Injector [pid = %5d] - Segment fault on mutatee side", getpid());
  }
  return Process::cbThreadContinue;
}

// Check whether a library is loaded
bool
SpInjector::IsLibraryLoaded(const char* libname) {
  char* name_only = sp_filename(libname);
  sp_debug("CHECKING - checking whether %s is loaded ...",
           name_only);

  std::string proc_maps = "";
  proc_maps += "/proc/";
  proc_maps += dt::itos(pid_);
  proc_maps += "/maps";
  const char* content = sp::GetFileText(proc_maps.c_str()).c_str();
  if (strstr(content, name_only)) {
    sp_debug("LOADED - %s is already loaded", name_only);
    return true;
  }
  sp_debug("NO LOADED - %s is not yet loaded", name_only);
  return false;
}

typedef struct {
  char libname[512];
  char err[512];
  char loaded;
  long pc;
  long sp;
  long bp;
} IjMsg;

// The main injection procedure, which has two main steps:
// Step 1: Force injectee to execute do_dlopen to load shared library
//         libijagent.so.
// Step 2: Force injectee to execute ij_agent in libijagent.so to load the
//         user-specified shared library. In this step, we have to pass
//         parameters by IPC mechanism, where we use shared memory in current
//         implementation.
//
// Q & A:
//
// 1. Why don't we directly use do_dlopen to load user-specified shared library?
//
// Answer: Because do_dlopen is not a public function, which is unsafe to use.
// Often times, do_dlopen causes injectee to crash, e.g., the library is not
// found. On the other hand, dlopen is safe to use, which would not causes the
// injectee process to crash.
//
// 2. To follow up question 1, since dlopen is safe to use, why don't we
// directly call dlopen in step 1?
//
// Answer: We are not allowed to do so. libc.so has sanity check on calling
// dlopen. If we call dlopen using IRPC, dlopen would fail, although it won't
// crash injectee process. In sum, the benefit of providing a level of
// indirection is, we can use unsafe do_dlopen function to load a controlled
// library libijagent.so, from libijagent.so, we use safe dlopen function to
// load uncontrolled user-provided library.
//
// 3. Why we use IPC mechanism to pass parameters to ij_agent in libijagent.so?
//
// Answer: ij_agent is a function that calls dlopen. After dlopen is invoked, we
// want to check whether the loading is successful. Even when dlopen fails, we
// also want to know the error message. Therefore, we need to do IPC for error
// report or checking return value. In this case, why don't we have an easy and
// uniformed way to pass argument and check return value?

void SpInjector::Inject(const char* lib_name) {

  // Verify the existence of lib_name
  char* abs_lib_name = realpath(lib_name, NULL);
  if (!abs_lib_name)
    sp_perror("Injector [pid = %5d] - cannot locate library %s.",
              getpid(), lib_name);

  if (IsLibraryLoaded(sp_filename(abs_lib_name))) {
    sp_print("Injector [pid = %5d]: Library %s is already loaded...",
             getpid(), sp_filename(lib_name));
    return;
  }

  // Step 1. Load libijagent.so
  if (!IsLibraryLoaded(IJAGENT)) {
    // Verify the existence of libijagent.so
    std::string ijagent(IJAGENT);
    StringSet ijagent_paths;
    if (!GetResolvedLibPath(ijagent, ijagent_paths))
      sp_perror("Injector [pid = %5d] - cannot find %s", getpid(), IJAGENT);

    // Inject libijagent.so
    LoadIjagent((*ijagent_paths.begin()).c_str());
  }

  // Step 2. Load user-specified library
  if (!IsLibraryLoaded(sp_filename(abs_lib_name))) {
    // Setup shared memory, this is to pass arguments to call dlopen in
    // libijagent.so
    IjMsg *shm = (IjMsg*)GetSharedMemory(IJMSG_ID, sizeof(IjMsg));
    strcpy(shm->libname, abs_lib_name);
    shm->err[0] = '\0';
    shm->loaded = -1;

    // Invoke dlopen in libijagent.so
    LoadUserLibrary();

    // Wait for dlopen to return
    int count = 0;
    while (shm->loaded == -1) {
      if (count > IJ_TIMEOUT) {
        sp_perror("INJECTOR [pid = %d]: injectee not response, abort", getpid());
      }
      sleep(1);
      ++count;
    }

    if (!shm->loaded) sp_perror(shm->err);
    else sp_print(shm->err);
  } else {
    sp_print("Injector [pid = %5d]: Library %s is already loaded...",
             getpid(), sp_filename(lib_name));
  }
  // proc_->detach();
}

// Invoke ijagent function in libijagent.so, which in turn invokes dlopen
void
SpInjector::LoadUserLibrary() {
  sp_debug("PAUSED - Process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("Injector [pid = %5d] - Failed to stop process %d",
              getpid(), pid_);
  }
  UpdateFrameInfo();
  Address ij_agent_addr = FindFunction("ij_agent");
  if (ij_agent_addr > 0) {
    sp_debug("FOUND - Address of ij_agent function at %lx", ij_agent_addr);
  }
  else {
    sp_perror("Injector [pid = %5d] - Failed to find ij_agent", getpid());
  }
  size_t size = GetIjTemplateSize();
  Address code_addr = proc_->mallocMemory(size);
  char* code = GetIjTemplate(ij_agent_addr, code_addr);
  sp_debug("ALLOCATED - Buffer for load-library code in mutatee's heap"
           " at %lx of %lu bytes", code_addr, (unsigned long)size);
  IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr);
  irpc->setStartOffset(2);
  sp_debug("POSTING - IRPC is on the way ...");
  if (!proc_->postIRPC(irpc)) {
    sp_perror("Injector [pid = %5d] - Failed to execute load-library code"
              " in mutatee's address space", getpid());
  }
  proc_->continueProc();
  Process::handleEvents(true);
  proc_->freeMemory(code_addr);
}

// Load a library by using do_dlopen
void
SpInjector::LoadIjagent(const char* lib_name) {
  // Make absolute path for this shared library
  char* libname = realpath(lib_name, NULL);
  if (!libname) {
    sp_perror("Injector [pid = %5d] - %s cannot be found",
              getpid(), sp_filename(lib_name));
  }
  sp_debug("PAUSED - Process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("Injector [pid = %5d] - Failed to stop process %d",
              getpid(), pid_);
  }
  UpdateFrameInfo();
  Process::registerEventCallback(EventType::Library, on_event_lib);
  Process::registerEventCallback(EventType::Signal, on_event_signal);

  // Find do_dlopen function
  Address do_dlopen_addr = FindFunction("do_dlopen");
  if (do_dlopen_addr > 0) {
    sp_debug("FOUND - Address of do_dlopen function at %lx", do_dlopen_addr);
  }
  else {
    sp_perror("Injector [pid = %5d] - Failed to find do_dlopen", getpid());
  }

  // Prepare irpc
  size_t lib_name_len = strlen(libname) + 1;
  Address lib_name_addr = proc_->mallocMemory(lib_name_len);
  proc_->writeMemory(lib_name_addr, (void*)libname, lib_name_len);
  sp_debug("STORED - Library name \"%s\" in mutatee's heap at %lx",
           sp_filename(libname), lib_name_addr);

  DlopenArg args;
  Address args_addr = proc_->mallocMemory(sizeof(DlopenArg));
  args.libname = (char*)lib_name_addr;
  args.mode = RTLD_NOW | RTLD_GLOBAL;
  args.link_map = 0;
  proc_->writeMemory(args_addr, &args, sizeof(args));
  sp_debug("STORED - do_dlopen's argument in mutatee's heap at %lx", args_addr);

  size_t size = GetCodeTemplateSize();
  Address code_addr = proc_->mallocMemory(size);
  char* code = GetCodeTemplate(args_addr, do_dlopen_addr, code_addr);
  sp_debug("ALLOCATED - Buffer for load-library code in mutatee's heap"
           " at %lx of %lu bytes", code_addr, (unsigned long)size);
  IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr);
  irpc->setStartOffset(2);

  if (!proc_->postIRPC(irpc)) {
    sp_perror("Injector [pid = %5d] - Failed to execute load-library code"
              " in mutatee's address space", getpid());
  }
  proc_->continueProc();
  Process::handleEvents(true);

  // Clean up
  proc_->freeMemory(lib_name_addr);
  proc_->freeMemory(args_addr);
  proc_->freeMemory(code_addr);
  free(libname);
}

// This piece of code is borrowed from DyninstAPI_RT
bool
SpInjector::GetResolvedLibPath(const std::string &filename,
                               StringSet &paths) {
  char *libPathStr, *libPath;
  std::vector<std::string> libPaths;
  struct stat dummy;
  FILE *ldconfig;
  char buffer[512];
  char *pos, *key, *val;

  // Prefer qualified file paths
  if (stat(filename.c_str(), &dummy) == 0) {
    paths.insert(filename);
  }

  // Search paths from mutatee's environment variables
  libPathStr = strdup(getenv("LD_LIBRARY_PATH"));
  libPath = strtok(libPathStr, ":");
  while (libPath != NULL) {
    libPaths.push_back(std::string(libPath));
    libPath = strtok(NULL, ":");
  }
  for (unsigned int i = 0; i < libPaths.size(); i++) {
    std::string str = libPaths[i] + "/" + filename;
    if (stat(str.c_str(), &dummy) == 0) {
      paths.insert(str);
    }
  }
  free(libPathStr);

  // Search ld.so.cache
  ldconfig = popen("/sbin/ldconfig -p", "r");
  if (ldconfig) {
    fgets(buffer, 512, ldconfig);       // ignore first line
    while (fgets(buffer, 512, ldconfig) != NULL) {
      pos = buffer;
      while (*pos == ' ' || *pos == '\t') pos++;
      key = pos;
      while (*pos != ' ') pos++;
      *pos = '\0';
      while (*pos != '=' && *(pos + 1) != '>') pos++;
      pos += 2;
      while (*pos == ' ' || *pos == '\t') pos++;
      val = pos;
      while (*pos != '\n' && *pos != '\0') pos++;
      *pos = '\0';
      if (strcmp(key, filename.c_str()) == 0) {
        paths.insert(val);
      }
    }
    pclose(ldconfig);
  }

  // Search hard-coded system paths
  libPaths.clear();
  libPaths.push_back("/usr/local/lib");
  libPaths.push_back("/usr/share/lib");
  libPaths.push_back("/usr/lib");
  libPaths.push_back("/usr/lib64");
  libPaths.push_back("/lib");
  libPaths.push_back("/lib64");
  std::string sp_path = getenv("SP_DIR");
  sp_path += "/";
  sp_path += getenv("PLATFORM");
  libPaths.push_back(sp_path);
  for (unsigned int i = 0; i < libPaths.size(); i++) {
    std::string str = libPaths[i] + "/" + filename;
    if (stat(str.c_str(), &dummy) == 0) {
      paths.insert(str);
    }
  }

  // Remove those libraries running on a different architecture from mutatee
  typedef std::vector<std::string> DepVector;
  DepVector vpaths;
  std::copy(paths.begin(), paths.end(), back_inserter(vpaths));
  for (DepVector::iterator j = vpaths.begin(); j != vpaths.end();) {
    sb::Symtab* obj = NULL;
    bool ret = sb::Symtab::openFile(obj, *j);
    if (ret) {
      if ((Address)obj->getAddressWidth() ==
          (Address)proc_->llproc()->getAddressWidth()) {
        j++;
      } else {
        j = vpaths.erase(j);
      }
    } else {
      j = vpaths.erase(j);
    }
  }
  paths.clear();
  std::copy(vpaths.begin(), vpaths.end(), inserter(paths, paths.begin()));
  return ( 0 < paths.size() );
}


void SpInjector::UpdateFrameInfo() {
  IjMsg* shm = (IjMsg*)GetSharedMemory(IJMSG_ID, sizeof(IjMsg));
  shm->pc = pc();
  shm->sp = sp();
  shm->bp = bp();
}

}
