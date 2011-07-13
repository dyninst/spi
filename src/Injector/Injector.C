#include "Injector.h"
#include "dlfcn.h"
#include "Symtab.h"
#include "Function.h"
#include "int_process.h"
#include "Event.h"
#include <signal.h>
#include "Common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using sp::Injector;
using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::IRPC;
using Dyninst::ProcControlAPI::LibraryPool;
using Dyninst::ProcControlAPI::Library;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
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
Dyninst::Address Injector::args_ = 0;
Process::cb_ret_t on_event_rpc(Event::const_ptr ev) {
  Process::ptr p = dyn_detail::boost::const_pointer_cast<Process>(ev->getProcess());
  IRPC::ptr r = dyn_detail::boost::const_pointer_cast<IRPC>(ev->getEventRPC()->getIRPC());
  Injector::verify_ret(p, (Dyninst::Address)Injector::args_);
  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_lib(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++)
    sp_debug("LOADED - %s", (*i)->getName().c_str());

  return Process::cbThreadContinue;
}

Process::cb_ret_t on_event_signal(Event::const_ptr ev) {
  EventSignal::const_ptr sigev = ev->getEventSignal();
  if(sigev->getSignal() == SIGSEGV) {
    sp_perror("segment fault on mutatee side");
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
    sp_perror("FATAL - %s not loaded", libname);
  }
  sp_print("INJECTED - %s", libname);
}

void Injector::verify_ret(Process::ptr proc, Dyninst::Address args_addr) {
  dlopen_args_t args;
  proc->readMemory(&args, args_addr, sizeof(Injector::dlopen_args_t));
  if (!args.link_map || (args.mode != (RTLD_NOW | RTLD_GLOBAL))) {
    sp_perror("wrong ret value from do_dlopen (link_map:%lx, mode:%x)",args.link_map, args.mode);
  } else {
    sp_debug("PASSED - correct ret value (link_map: %lx, mode: %x)", args.link_map, args.mode);
  }
}

typedef struct {
  char libname[512];
  char err[512];
} IjMsg;

void Injector::inject(const char* lib_name) {
  // verify the path of lib_name
  char* abs_lib_name = realpath(lib_name, NULL);
  if (!abs_lib_name) sp_print("cannot locate %s", abs_lib_name);

  // check the existence of libijagent.so
  std::string ijagent("libijagent.so");
  DepNames ijagent_paths;
  if (!get_resolved_lib_path(ijagent, ijagent_paths))
    sp_perror("cannot find libijagent.so");

  // setup shared memory memory 1986
  const int SHMSZ = sizeof(IjMsg);
  key_t key = 1986;
  int shmid;
  if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
    sp_perror("failed to creat a shared memory w/ size %d bytes", SHMSZ);
  }
  IjMsg *shm;
  if ((long)(shm = (IjMsg*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("failed to get shared memory pointer");
  }
  strcpy(shm->libname, abs_lib_name);
  shm->err[0] = '\0';

  // inject libijagent.so
  inject_internal((*ijagent_paths.begin()).c_str());

  // invoke load_lib

  // wait, should setup an alarm mechanism 
  while (strlen(shm->err) <= 0)
    sleep(1);
  sp_print(shm->err);

  // Verify
  verify_lib_loaded(proc_, abs_lib_name);
}

/* Inject one library for each invokation. */
void Injector::inject_internal(const char* lib_name) {
  // Make absolute path for this shared library
  char* libname = realpath(lib_name, NULL);
  if (!libname) sp_perror("%s cannot be found", lib_name);

  // Stop mutatee and register events
  sp_debug("process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("failed to stop process %d", pid_);
  }
  Process::registerEventCallback(EventType::RPC, on_event_rpc);
  Process::registerEventCallback(EventType::Library, on_event_lib);
  Process::registerEventCallback(EventType::Signal, on_event_signal);

  // Find do_dlopen function
  Dyninst::Address do_dlopen_addr = find_do_dlopen();
  if (do_dlopen_addr > 0) {
    sp_debug("find the address of do_dlopen function at %lx", do_dlopen_addr);
  }
  else {
    sp_perror("failed to find do_dlopen");
  }

  // Prepare irpc
  size_t lib_name_len = strlen(libname) + 1;
  Dyninst::Address lib_name_addr = proc_->mallocMemory(lib_name_len);
  proc_->writeMemory(lib_name_addr, (void*)libname, lib_name_len);
  sp_debug("store library name \"%s\" in mutatee's heap at %lx", libname, lib_name_addr);

  dlopen_args_t args;
  Dyninst::Address args_addr = proc_->mallocMemory(sizeof(dlopen_args_t));
  args.libname = (char*)lib_name_addr;
  args.mode = RTLD_NOW | RTLD_GLOBAL;
  args.link_map = 0;
  proc_->writeMemory(args_addr, &args, sizeof(args));
  args_ = args_addr;
  sp_debug("store do_dlopen's argument in mutatee's heap at %lx", args_addr);

  size_t size = get_code_tmpl_size();
  Dyninst::Address code_addr = proc_->mallocMemory(size);
  char* code = get_code_tmpl(args_addr, do_dlopen_addr, code_addr);
  sp_debug("allocate a buffer for load-library code in mutatee's heap"
           " at %lx of %d bytes", code_addr, size);
  IRPC::ptr irpc = IRPC::createIRPC(code, size, code_addr);
  irpc->setStartOffset(2);

  // Post all irpcs
  if (!thr_->postIRPC(irpc)) {
    sp_perror("failed to execute load-library code in mutatee's address space");
  }

  // Wait for finish
  while (thr_->isStopped()) {
    thr_->continueThread();
    Process::handleEvents(true);
  }


  // Clean up
  proc_->freeMemory(lib_name_addr);
  proc_->freeMemory(args_addr);
  proc_->freeMemory(code_addr);
  free(libname);
  proc_->detach();
}

bool Injector::get_resolved_lib_path(const std::string &filename, DepNames &paths) {
  char *libPathStr, *libPath;
  std::vector<std::string> libPaths;
  struct stat dummy;
  FILE *ldconfig;
  char buffer[512];
  char *pos, *key, *val;

  // prefer qualified file paths
  // TODO: what if filename is ./agent
  if (stat(filename.c_str(), &dummy) == 0) {
    paths.insert(filename);
  }

  // search paths from mutatee's environment variables
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

  // search ld.so.cache
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

  // search hard-coded system paths
  libPaths.clear();
  libPaths.push_back("/usr/local/lib");
  libPaths.push_back("/usr/share/lib");
  libPaths.push_back("/usr/lib");
  libPaths.push_back("/usr/lib64");
  libPaths.push_back("/lib");
  libPaths.push_back("/lib64");
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
    Symtab* obj = NULL;
    bool ret = Symtab::openFile(obj, *j);
    if (ret) {
      if (obj->getAddressWidth() == proc_->llproc()->getAddressWidth()) {
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

/* Here we go! */
int main(int argc, char *argv[]) {
  if (argc != 3) {
    sp_print("usage: %s PID LIB_NAME", argv[0]);
    exit(0);
  }

  Dyninst::PID pid = atoi(argv[1]);
  const char* lib_name = argv[2];

  sp_print("INJECTING - %s ...", lib_name);
  Injector::ptr injector = Injector::create(pid);
  injector->inject(lib_name);

  return 0;
}
