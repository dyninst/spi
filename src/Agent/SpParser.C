#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "SpParser.h"
#include "SpCommon.h"

#include "PatchMgr.h"
#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "CodeObject.h"

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

    if (lib_lookup.find(load_addr) == lib_lookup.end()) continue;

    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();

    PatchObject* patch_obj = PatchObject::create(co, load_addr);
    patch_objs.push_back(patch_obj);
    if (sym->isExec()) {
      sp_debug("FOUND - Executable %s at %lx", sp_filename(sym->name().c_str()), load_addr);
      exe_obj_ = patch_obj;
    } else {
      sp_debug("FOUND - Library %s at %lx", sp_filename(sym->name().c_str()), load_addr);
    }
  }
  shmctl(IJLIB_ID, IPC_RMID, NULL);

  // initialize PatchAPI objects
  AddrSpacePtr as = AddrSpace::create(exe_obj_);
  mgr_ = PatchMgr::create(as);
  for (SpParser::PatchObjects::iterator i = patch_objs.begin(); i != patch_objs.end(); i++) {
    if (*i != exe_obj_) {
      as->loadObject(*i);
    }
  }
  return mgr_;
}

/* Find the function that contains addr */
Dyninst::ParseAPI::Function* SpParser::findFunction(Dyninst::Address addr) {
  AddrSpacePtr as = mgr_->as();
  for (AddrSpace::ObjSet::iterator ci = as->objSet().begin(); ci != as->objSet().end(); ci++) {
    PatchObject* obj = *ci;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    Dyninst::Address upper_bound = obj->codeBase() + cs->length();
    Dyninst::Address lower_bound = obj->codeBase();
    if (addr >= lower_bound && addr <= upper_bound) {
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
          sp_debug("FOUND - Function %s", (*funcs.begin())->name().c_str());
          break;
        }
      }
      break;
    }
  }
  return NULL;
}
