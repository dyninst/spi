#include "SpInjector.h"
#include "dlfcn.h"
#include "Symtab.h"
#include "AddrLookup.h"
#include "Type.h"
#include "Function.h"
#include "int_process.h"
#include "Event.h"
#include <signal.h>
#include "SpCommon.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using sp::SpInjector;
using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::IRPC;
using Dyninst::ProcControlAPI::LibraryPool;
using Dyninst::ProcControlAPI::Library;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
using Dyninst::SymtabAPI::Function;
using Dyninst::SymtabAPI::AddressLookup;

/* Constructor */
SpInjector::ptr SpInjector::create(Dyninst::PID pid) {
  SpInjector::ptr ret = ptr(new SpInjector(pid));
  return ret;
}

SpInjector::SpInjector(Dyninst::PID pid) : pid_(pid) {
  proc_ = Process::attachProcess(pid);
  if (!proc_) {
    sp_perror("failed to attach to process %d.", pid);
  }
  ThreadPool& thrs = proc_->threads();
  thr_ = thrs.getInitialThread();
}

SpInjector::~SpInjector() {
  proc_->detach();
}

/* Find do_dlopen */
Dyninst::Address SpInjector::find_func(char* func) {
  LibraryPool& libs = proc_->libraries();
  Library::ptr lib = Library::ptr();
  for (LibraryPool::iterator li = libs.begin(); li != libs.end(); li++) {
    std::string name = (*li)->getName();
    if (name.size() <= 0) continue;
    Symtab *obj = NULL;
    bool ret = Symtab::openFile(obj, name);
    std::vector <Function *> funcs;
    if (!obj || !ret) {
      sp_perror("failed to open %s", name.c_str());
    } else {
      obj->findFunctionsByName(funcs, func);
    }
    if (funcs.size() > 0) {
      return funcs[0]->getOffset() + (*li)->getLoadAddress();
    }
  }
  return 0;
}


/* Event handlers */
static Dyninst::Address ijagent_load_addr = 0;
Process::cb_ret_t on_event_ijagent(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  Process::const_ptr proc = libev->getProcess();
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++) {
    sp_debug("LOADED - %s @ %lx", (*i)->getName().c_str(), (*i)->getLoadAddress());
    ijagent_load_addr = (*i)->getLoadAddress();
  }
}

Process::cb_ret_t on_event_lib(Event::const_ptr ev) {
  EventLibrary::const_ptr libev = ev->getEventLibrary();
  const std::set<Library::ptr> &libs = libev->libsAdded();
  Process::const_ptr proc = libev->getProcess();

  static int lib_num = 0;
  // the global variable ij_lib_load_addrs's address
  Dyninst::Address ij_lib_addr;

  Symtab *obj = NULL;
  bool err = Symtab::openFile(obj, IJAGENT);
  if (!err || !obj) {
    sp_debug("WARNING: failed to open %s", IJAGENT);
  } else {
    std::vector<Symbol*> symbols;
    obj->findSymbol(symbols, "ij_lib_load_addrs");
    if (symbols.size() == 0) {
      sp_debug("WARNING: failed to find symbol %s, %d symbols found", "ij_lib_load_addrs", symbols.size());
    } else {
      ij_lib_addr = symbols[0]->getOffset() + ijagent_load_addr;
      sp_debug("get %s at %lx", symbols[0]->getPrettyName().c_str(), ij_lib_addr);
    }
  }
  for (std::set<Library::ptr>::iterator i = libs.begin(); i != libs.end(); i++) {
    Dyninst::Address loaded_addr = (*i)->getLoadAddress();
    sp_debug("LOADED - %s @ %lx", (*i)->getName().c_str(), loaded_addr);
    proc->writeMemory(ij_lib_addr + sizeof(Dyninst::Address) * lib_num,
                      &loaded_addr, sizeof(Dyninst::Address));
    Dyninst::Address end = 0;
    proc->writeMemory(ij_lib_addr + sizeof(Dyninst::Address) * (lib_num+1),
                      &end, sizeof(Dyninst::Address));
    ++lib_num;
  }

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
void SpInjector::verify_lib_loaded(const char* libname) {
  bool found = false;
  int count = 0;
  char* name_only = strrchr(libname, '/');
  if (!name_only)  name_only = (char*)libname;

  do {
    LibraryPool& libs = proc_->libraries();
    LibraryPool::iterator li = libs.begin();
    for (li = libs.begin(); li != libs.end(); li++) {
      std::string name = (*li)->getName();
      if (name.size() <= 0) continue;
      sp_debug("CHECKING %s", name.c_str());
      if (name.find(name_only) != std::string::npos) {
        found = true;
        break;
      }
    }
    if (!found) {
      sp_print("Injector [pid = %5d]: not loaded, sleep 1 second, and check again",
               getpid());
      sleep(1);
      ++count;
    }
  } while (!found && count < 10);

  if (found)
    sp_print("Injector [pid = %5d]: INJECTED, confirmed", getpid());
  else
    sp_print("Injector [pid = %5d]: still not loaded, abort", getpid());
}

typedef struct {
  char libname[512];
  char err[512];
  char loaded;
} IjMsg;

typedef struct {
  Dyninst::Address offsets[100];
  size_t sizes[100];
} IjLib;

void SpInjector::inject(const char* lib_name) {
  // verify the path of lib_name
  char* abs_lib_name = realpath(lib_name, NULL);
  if (!abs_lib_name) sp_print("cannot locate %s", abs_lib_name);

  // check the existence of libijagent.so
  std::string ijagent(IJAGENT);
  DepNames ijagent_paths;
  if (!get_resolved_lib_path(ijagent, ijagent_paths))
    sp_perror("cannot find libijagent.so");

  // setup shared memory 1986
  const int SHMSZ = sizeof(IjMsg);
  key_t key = IJMSG_ID;
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
  shm->loaded = -1;

  // setup shared memory 1985
  const int SHLIBZ = sizeof(IjLib);
  key_t key_lib = IJLIB_ID;
  int shmid_lib;
  if ((shmid_lib = shmget(key_lib, SHLIBZ, IPC_CREAT | 0666)) < 0) {
    sp_perror("failed to creat a shared memory w/ size %d bytes", SHLIBZ);
  }
  IjLib *shm_lib;
  if ((long)(shm_lib = (IjLib*)shmat(shmid_lib, NULL, 0)) == (long)-1) {
    sp_perror("failed to get shared memory pointer");
  }
  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);
  for (int i = 0; i < tabs.size(); i++) {
    Symtab* sym = tabs[i];
    Dyninst::Address load_addr;
    al->getLoadAddress(sym, load_addr);
    shm_lib->offsets[i] = (load_addr?load_addr:sym->getLoadAddress());
    shm_lib->sizes[i] = sym->imageLength();
  }
  shm_lib->offsets[tabs.size()] = -1;
  shm_lib->sizes[tabs.size()] = -1;

  // inject libijagent.so
  inject_internal((*ijagent_paths.begin()).c_str());

  // invoke load_lib
  invoke_ijagent();

  // wait, should setup an alarm mechanism 
  int count = 0;
  while (shm->loaded == -1) {
    if (count > 5) {
      sp_perror("INJECTOR [pid = %d]: injectee not response, abort", getpid());
    }
    sleep(1);
    ++count;
  }
  if (!shm->loaded) sp_perror(shm->err);

  // Verify
  sp_print(shm->err);
  verify_lib_loaded(lib_name);
}

void SpInjector::invoke_ijagent() {
  sp_debug("process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("failed to stop process %d", pid_);
  }
  Process::registerEventCallback(EventType::Library, on_event_lib);

  Dyninst::Address ij_agent_addr = find_func("ij_agent");
  if (ij_agent_addr > 0) {
    sp_debug("find the address of ij_agent function at %lx", ij_agent_addr);
  }
  else {
    sp_perror("failed to find ij_agent");
  }

  size_t size = get_ij_tmpl_size();
  Dyninst::Address code_addr = proc_->mallocMemory(size);
  char* code = get_ij_tmpl(ij_agent_addr, code_addr);
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

  proc_->freeMemory(code_addr);
}

/* Inject one library for each invokation. */
void SpInjector::inject_internal(const char* lib_name) {
  // Make absolute path for this shared library
  char* libname = realpath(lib_name, NULL);
  if (!libname) sp_perror("%s cannot be found", lib_name);

  // Stop mutatee and register events
  sp_debug("process %d is paused by injector.", pid_);
  if (!proc_->stopProc()) {
    sp_perror("failed to stop process %d", pid_);
  }
  Process::registerEventCallback(EventType::Library, on_event_ijagent);
  Process::registerEventCallback(EventType::Signal, on_event_signal);

  // Find do_dlopen function
  Dyninst::Address do_dlopen_addr = find_func("do_dlopen");
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
}


bool SpInjector::get_resolved_lib_path(const std::string &filename, DepNames &paths) {
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

  sp_print("Injector [pid = %5d]: INJECTING - %s ...", getpid(), lib_name);
  SpInjector::ptr injector = SpInjector::create(pid);
  injector->inject(lib_name);

  return 0;
}
