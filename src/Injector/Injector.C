#include "Injector.h"
#include "dlfcn.h"
#include "Symtab.h"
#include "Function.h"
#include "int_process.h"
#include "Event.h"
#include <signal.h>

using sp::Injector;
using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::IRPC;
using Dyninst::ProcControlAPI::LibraryPool;
using Dyninst::ProcControlAPI::Library;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Function;

/* Debugging facility */
#define print_reg(thr, reg) do { \
  Dyninst::MachRegisterVal reg; \
  thr->getRegister(x86::reg, reg); \
  fprintf(stderr, "** %s = %x\n", #reg, reg); \
} while (0)

static void dump_registers(Thread::const_ptr thr) {
  print_reg(thr, esp);
  print_reg(thr, eip);
  print_reg(thr, eax);
  print_reg(thr, ebx);
  print_reg(thr, ecx);
  print_reg(thr, edx);
}

/* Constructor */
Injector::ptr Injector::create(Dyninst::PID pid) {
  Injector::ptr ret = ptr(new Injector(pid));
  return ret;
}
Injector::Injector(Dyninst::PID pid) : pid_(pid) {
  proc_ = Process::attachProcess(pid);
  if (!proc_) {
    fprintf(stderr, "ERROR: failed to attach to process %d."
                    "Injector exits!\n", pid);
    exit(0);
  }
}

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

/* Find do_dlopen */
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
    if (funcs.size() > 0) return funcs[0]->getOffset();
  }
  return 0;
}

Process::cb_ret_t on_event_rpc(Event::const_ptr ev) {
  dump_registers(ev->getThread());
  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_lib(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++)
    printf("The library %s is loaded successfully!\n", (*i)->getName().c_str());
  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_signal(Event::const_ptr ev) {
  EventSignal::const_ptr sigev = ev->getEventSignal();
  if(sigev->getSignal() == SIGSEGV) {
    fprintf(stderr, "ERROR: segment fault on mutatee side\n");
    dump_registers(ev->getThread());
  }
  return Process::cbDefault;
}

/* The main inject procedure.
   The fault handling is simple, simply report the error and exit! */
void Injector::inject(const char* lib_name) {

  printf("Step 1, Process %d is paused by injector.\n", pid_);
  if (!proc_->stopProc()) {
    fprintf(stderr, "ERROR: failed to stop process %d\n", pid_);
    exit(0);
  }

  printf("Step 2, Find the address of do_dlopen function ");
  Dyninst::Address do_dlopen_addr = find_do_dlopen();
  if (do_dlopen_addr > 0) printf("at %x\n", do_dlopen_addr);
  else {
    fprintf(stderr, "ERROR: failed to find do_dlopen\n");
    exit(0);
  }

  printf("Step 3, Store library name \"%s\" in mutatee's heap ", lib_name);
  size_t lib_name_len = strlen(lib_name) + 1;
  Dyninst::Address lib_name_addr = proc_->mallocMemory(lib_name_len);
  proc_->writeMemory(lib_name_addr, (void*)lib_name, lib_name_len);
  printf("at %x\n", lib_name_addr);

  printf("Step 4, Store do_dlopen's argument in mutatee's heap ");
  dlopen_args_t args;
  Dyninst::Address args_addr = proc_->mallocMemory(sizeof(args));
  args.libname = (char*)lib_name_addr;
  args.mode = RTLD_NOW | RTLD_GLOBAL;
  args.link_map = 0;
  proc_->writeMemory(args_addr, &args, sizeof(args));
  printf("at %x\n", args_addr);

  printf("Step 5, Setup load-library code and write code into mutatee's heap ");
  size_t size = get_code_tmpl_size();
  Dyninst::Address code_addr = proc_->mallocMemory(size);
  char* code = get_code_tmpl(args_addr, do_dlopen_addr, code_addr);
  printf("at %x of %d bytes\n", code_addr, size);

  printf("Step 6, Force the mutatee to execute load-library code at %x\n", code_addr);
  Process::registerEventCallback(EventType::RPC, on_event_rpc);
  Process::registerEventCallback(EventType::Library, on_event_lib);
  Process::registerEventCallback(EventType::Signal, on_event_signal);
  IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr);
  ThreadPool& thrs = proc_->threads();
  Thread::ptr t = thrs.getInitialThread();
  dump_registers(t);
  irpc->setStartOffset(2);
  if (!t->postIRPC(irpc)) {
    fprintf(stderr, "ERROR: failed to execute load-library code in mutatee's address space\n");
    exit(0);
  }

  // Wait for completion
  while (t->isStopped()) {
    t->continueThread();
    Process::handleEvents(true);
  }

  proc_->freeMemory(lib_name_addr);
  proc_->freeMemory(args_addr);
  proc_->freeMemory(code_addr);
  proc_->detach();
}

/* Here we go! */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s PID LIB_NAME\n", argv[0]);
    exit(0);
  }

  Dyninst::PID pid = atoi(argv[1]);
  const char* lib_name = argv[2];

  fprintf(stdout, "Injecting library %s to process %d ...\n", lib_name, pid);
  Injector::ptr injector = Injector::create(pid);
  injector->inject(lib_name);

  return 0;
}
