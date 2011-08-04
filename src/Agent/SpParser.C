#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "SpInstrumenter.h"
#include "SpParser.h"
#include "SpCommon.h"

#include "Point.h"
#include "PatchMgr.h"
#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "CodeObject.h"

using sp::SpInstrumenter;
using sp::SpParser;
using Dyninst::SymtabAPI::AddressLookup;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::ParseAPI::SymtabCodeSource;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::PointMakerPtr;


SpParser::SpParser() : exe_obj_(NULL) {
}

SpParser::~SpParser() {
  for (CodeSources::iterator i = code_srcs_.begin(); i != code_srcs_.end(); i++) {
    SymtabCodeSource* scs = static_cast<SymtabCodeSource*>(*i);
    delete scs;
  }
  for (CodeObjects::iterator i = code_objs_.begin(); i != code_objs_.end(); i++)
    delete *i;
}

SpParser::ptr SpParser::create() {
  return ptr(new SpParser());
}

/* Default implementation is runtime parsing. */
typedef struct {
  Dyninst::Address offsets[100];
} IjLib;

PatchMgrPtr SpParser::parse() {
  int shmid;
  key_t key = 1985;
  IjLib* shm;
  if ((shmid = shmget(key, sizeof(IjLib), 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  if ((char*)(shm = (IjLib*)shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  // build lookup map
  int cur = 0;
  typedef std::map<Dyninst::Address, bool> LibLookup;
  LibLookup lib_lookup;
  while (shm->offsets[cur] != -1) {
    lib_lookup[shm->offsets[cur]] = true;
    ++cur;
  }

  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);
  sp_debug("FOUND - %d symtabs", tabs.size());

  PatchObjects patch_objs;
  for (std::vector<Symtab*>::iterator i = tabs.begin(); i != tabs.end(); i++) {
    Symtab* sym = *i;
    Dyninst::Address load_addr = 0;
    al->getLoadAddress(sym, load_addr);
    if (!load_addr) load_addr = sym->getLoadAddress();

    if ((lib_lookup.find(load_addr) == lib_lookup.end())     &&
        (sym->name().find(get_agent_name()) == string::npos) &&
        (sym->name().find("libagent.so") == string::npos)) {
        continue;
    }

    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();

    PatchObject* patch_obj = PatchObject::create(co, load_addr);
    patch_objs.push_back(patch_obj);
    if (sym->isExec()) {
      sp_debug("PARSED - Executable %s at %lx", sp_filename(sym->name().c_str()), load_addr);
      exe_obj_ = patch_obj;
    } else {
      sp_debug("PARSED - Library %s at %lx", sp_filename(sym->name().c_str()), load_addr);
    }
  }

  // initialize PatchAPI objects
  AddrSpacePtr as = AddrSpace::create(exe_obj_);
  PointMakerPtr pf = PointMakerPtr(new PointMaker);
  SpInstrumenter::ptr inst = SpInstrumenter::create(as);
  mgr_ = PatchMgr::create(as, pf, inst);
  for (SpParser::PatchObjects::iterator i = patch_objs.begin(); i != patch_objs.end(); i++) {
    if (*i != exe_obj_) {
      as->loadObject(*i);
    }
  }

  // destroy shared memory
  shmctl(IJLIB_ID, IPC_RMID, NULL);
  shmctl(IJMSG_ID, IPC_RMID, NULL);

  return mgr_;
}

/* Find the function that contains addr */
PatchFunction* SpParser::findFunction(Dyninst::Address addr) {
  AddrSpacePtr as = mgr_->as();
  for (AddrSpace::ObjSet::iterator ci = as->objSet().begin(); ci != as->objSet().end(); ci++) {
    PatchObject* obj = *ci;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    Dyninst::Address upper_bound = obj->codeBase() + cs->length();
    Dyninst::Address lower_bound = obj->codeBase();
    if (addr >= lower_bound && addr <= upper_bound) {
      sp_debug("FOUND - In range [%lx, %lx) in library %s",
              lower_bound, upper_bound, sp_filename(sym->name().c_str()));

      Dyninst::Address address = addr;
      Dyninst::SymtabAPI::Function* f;
      if (!sym->getContainingFunction(address, f)) {
        address -= lower_bound;
      }

      for (std::vector<CodeRegion*>::const_iterator ri = cs->regions().begin();
           ri != cs->regions().end(); ri++) {
        std::set<ParseAPI::Function*> funcs;
        obj->co()->findFuncs(*ri, address, funcs);
        if (funcs.size() > 0) {
          PatchFunction* pfunc = obj->getFunc(*funcs.begin());
          sp_debug("FOUND - Function %s", pfunc->name().c_str());
          return pfunc;
        }
      }
      break;
    }
  }
  return NULL;
}

void* SpParser::get_payload(string payload_name) {
  sp_debug("PAYLOAD - looking for %s", payload_name.c_str());
  AddrSpacePtr as = mgr_->as();
  for (AddrSpace::ObjSet::iterator ci = as->objSet().begin(); ci != as->objSet().end(); ci++) {
    PatchObject* obj = *ci;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();

    std::vector<Symbol*> symbols;
    if (sym->findSymbol(symbols, payload_name)) {
      sp_debug("PAYLOAD - get payload");
    }
  }
}

typedef struct {
  char libname[512];
  char err[512];
  char loaded;
} IjMsg;

char* SpParser::get_agent_name() {
  int shmid;
  key_t key = 1986;
  IjMsg* msg_shm;
  if ((shmid = shmget(key, sizeof(IjMsg), 0666)) < 0) {
    sp_perror("FATAL - Failed to get agent shared library name");
  }
  if ((char*)(msg_shm = (IjMsg*)shmat(shmid, NULL, 0)) == (char *) -1) {
    sp_perror("FATAL - Failed to get agent library name");
  }
  return msg_shm->libname;
}
