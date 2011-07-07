#include "Injector.h"
#include "dlfcn.h"
#include "Symtab.h"
#include "Function.h"
#include "int_process.h"
#include "Event.h"
#include <signal.h>
#include "Common.h"

using sp::Injector;
using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::IRPC;
using Dyninst::ProcControlAPI::LibraryPool;
using Dyninst::ProcControlAPI::Library;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Function;

/* Constructor */
Injector::ptr Injector::create(Dyninst::PID pid) {
  Injector::ptr ret = ptr(new Injector(pid));
  return ret;
}
Injector::Injector(Dyninst::PID pid) : pid_(pid) {
  proc_ = Process::attachProcess(pid);
  if (!proc_) {
    sp_perror("failed to attach to process %d.", pid);
  }
  ThreadPool& thrs = proc_->threads();
  thr_ = thrs.getInitialThread();
}

/* Find do_dlopen */
Library::ptr Injector::find_lib(Process::ptr proc, char* libname) {
  LibraryPool& libs = proc->libraries();
  Library::ptr lib = Library::ptr();
  for (LibraryPool::iterator li = libs.begin(); li != libs.end(); li++) {
    std::string name = (*li)->getName();
    if (name.find(libname) != std::string::npos) {
      lib = *li;
      break;
    }
  }
  return lib;
}

Dyninst::Address Injector::find_do_dlopen() {
  LibraryPool& libs = proc_->libraries();
  Library::ptr lib = Library::ptr();
  for (LibraryPool::iterator li = libs.begin(); li != libs.end(); li++) {
    std::string name = (*li)->getName();
    if (name.size() <= 0) continue;
    Symtab *obj = NULL;
    bool err = Symtab::openFile(obj, name);
    std::vector <Function *> funcs;
    obj->findFunctionsByName(funcs, "do_dlopen");
    if (funcs.size() > 0) {
      return funcs[0]->getOffset() + (*li)->getLoadAddress();
    }
  }
  return 0;
}

/* Event handlers */
Injector::dlopen_args_t* Injector::args_ = NULL;
char* Injector::libname_ = NULL;

Process::cb_ret_t on_event_rpc(Event::const_ptr ev) {
  sp_debug("Step 7. load-library code completed, validate the result ...");
  Process::ptr p = dyn_detail::boost::const_pointer_cast<Process>(ev->getProcess());
  Injector::verify_ret(p, (Dyninst::Address)Injector::args_);
  Injector::verify_lib_loaded(p, Injector::libname_);
  sp_print("%s is loaded", Injector::libname_);
  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_lib(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++)
    sp_debug("*** The library %s is loaded the first time", (*i)->getName().c_str());
  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_signal(Event::const_ptr ev) {
  EventSignal::const_ptr sigev = ev->getEventSignal();
  if(sigev->getSignal() == SIGSEGV) {
    sp_perror("segment fault on mutatee side");
    //TODO(wenbin): should resume mutatee's execution
  }
  return Process::cbThreadContinue;
}

/* check whether we load the library successfully */
void Injector::verify_lib_loaded(Process::ptr proc, char* libname) {
  LibraryPool& libs = proc->libraries();
  bool found = false;
  LibraryPool::iterator li = libs.begin();
  for (li = libs.begin(); li != libs.end(); li++) {
    std::string name = (*li)->getName();
    if (name.size() <= 0) continue;
    if (name.compare(libname) == 0) {
      found = true;
      break;
    }
  }
  if (!found) {
    sp_perror("library %s is not loaded", libname);
  }
}

void Injector::verify_ret(Process::ptr proc, Dyninst::Address args_) {
  dlopen_args_t args;
  proc->readMemory(&args, args_, sizeof(Injector::dlopen_args_t));
  if (!args.link_map || (args.mode != (RTLD_NOW | RTLD_GLOBAL))) {
    sp_perror("error return value from do_dlopen function");
  }
}

/* The main inject procedure. */
void Injector::inject(const char* lib_name) {

  // 0, Check the existence of lib_name
  libname_ = realpath(lib_name, NULL);
  if (!libname_) {
    sp_perror("invalid path for library %s", lib_name);
  }
  sp_debug("Step 0, absolute path of this lib is: %s", libname_);

  // 1. Stop process, register some event handlers
  sp_debug("Step 1, Process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("failed to stop process %d", pid_);
  }
  Process::registerEventCallback(EventType::RPC, on_event_rpc);
  Process::registerEventCallback(EventType::Library, on_event_lib);
  Process::registerEventCallback(EventType::Signal, on_event_signal);

  // 2. Find do_dlopen function
  Dyninst::Address do_dlopen_addr = find_do_dlopen();
  if (do_dlopen_addr > 0) {
    sp_debug("Step 2, Find the address of do_dlopen function at %lx", do_dlopen_addr);
  }
  else {
    sp_perror("failed to find do_dlopen");
  }

  // 3. Store library name to mutatee process's heap
  size_t lib_name_len = strlen(libname_) + 1;
  Dyninst::Address lib_name_addr = proc_->mallocMemory(lib_name_len);
  proc_->writeMemory(lib_name_addr, (void*)libname_, lib_name_len);
  sp_debug("Step 3, Store library name \"%s\" in mutatee's heap at %lx", libname_, lib_name_addr);

  // 4. Store argument of do_dlopen to mutatee process's heap
  dlopen_args_t args;
  Dyninst::Address args_addr = proc_->mallocMemory(sizeof(dlopen_args_t));
  args_ = (dlopen_args_t*)args_addr;
  args.libname = (char*)lib_name_addr;
  args.mode = RTLD_NOW | RTLD_GLOBAL;
  args.link_map = 0;
  proc_->writeMemory((Dyninst::Address)args_, &args, sizeof(args));
  sp_debug("Step 4, Store do_dlopen's argument in mutatee's heap at %lx", args_);

  // 5. Allocate a buffer for load-library code in mutatee process's heap
  size_t size = get_code_tmpl_size();
  Dyninst::Address code_addr = proc_->mallocMemory(size);
  char* code = get_code_tmpl((Dyninst::Address)args_, do_dlopen_addr, code_addr);
  sp_debug("Step 5, Allocate a buffer for load-library code in mutatee's heap"
           " at %lx of %d bytes", code_addr, size);

  // 6, IRPC!
  sp_debug("Step 6, Force the mutatee to execute load-library code at %lx", code_addr);
  IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr);
  irpc->setStartOffset(2);
  if (!thr_->postIRPC(irpc)) {
    sp_perror("failed to execute load-library code in mutatee's address space");
  }

  // 7. Wait for completion
  while (thr_->isStopped()) {
    thr_->continueThread();
    Process::handleEvents(true);
  }

  // 9. Clean up
  proc_->freeMemory(lib_name_addr);
  proc_->freeMemory((Dyninst::Address)args_);
  proc_->freeMemory(code_addr);
  free(libname_);

  proc_->detach();
}

/* Here we go! */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    sp_print("usage: %s PID LIB_NAME", argv[0]);
    exit(0);
  }

  Dyninst::PID pid = atoi(argv[1]);
  const char* lib_name = argv[2];

  sp_print("Injecting library %s to process %d ...", lib_name, pid);
  Injector::ptr injector = Injector::create(pid);
  injector->inject(lib_name);

  return 0;
}
