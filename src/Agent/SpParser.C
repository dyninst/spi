#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "SpInstrumenter.h"
#include "SpParser.h"
#include "SpCommon.h"
#include "SpContext.h"
#include "SpAddrSpace.h"

#include "Point.h"
#include "PatchMgr.h"
#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "CodeObject.h"

using sp::SpParser;
using sp::SpAddrSpace;
using Dyninst::SymtabAPI::AddressLookup;
using Dyninst::SymtabAPI::Symtab;
using Dyninst::SymtabAPI::Symbol;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::ParseAPI::SymtabCodeSource;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::AddrSpace;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;


SpParser::SpParser()
  : exe_obj_(NULL) {
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
  return ptr(new SpParser);
}

/* Default implementation is runtime parsing. */
typedef struct {
  Dyninst::Address offsets[100];
} IjLib;

PatchMgrPtr SpParser::parse() {
  if (mgr_) return mgr_;

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
  lib_lookup[0] = true;
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
    sp_debug("load_addr: %lx, name: %s", load_addr, sym->name().c_str());
    // if (!load_addr) load_addr = sym->getLoadAddress();
    //Symtab::getLoadAddress
    //AddressLookup::getLoadAddress
    //PatchObject::codeBase
    if ((lib_lookup.find(load_addr) == lib_lookup.end())     &&
        (sym->name().find(sp_filename(sp_filename(get_agent_name()))) == string::npos) &&
        (sym->name().find("libagent.so") == string::npos)) {
      sp_debug("SKIP - parsing %s", sp_filename(sym->name().c_str()));
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

  assert(exe_obj_);
  // initialize PatchAPI objects
  AddrSpace* as = SpAddrSpace::create(exe_obj_);
  //Dyninst::PatchAPI::Instrumenter* inst = sp::TrapInstrumenter::create(as);
  Dyninst::PatchAPI::Instrumenter* inst = sp::JumpInstrumenter::create(as);
  mgr_ = PatchMgr::create(as, inst);
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
  assert(0);
  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {
    PatchObject* obj = ci->second;
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
        std::set<Dyninst::ParseAPI::Function*> funcs;
        obj->co()->findFuncs(*ri, address, funcs);
        if (funcs.size() > 0) {
          PatchFunction* pfunc = obj->getFunc(*funcs.begin());
          sp_debug("FOUND - Function %s with %d callees", pfunc->name().c_str(), pfunc->calls().size());
          return pfunc;
        }
      }
      break;
    }
  }
  return NULL;
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
  sp_debug("GET AGENT NAME - %s", msg_shm->libname);
  return msg_shm->libname;
}

Dyninst::Address SpParser::get_func_addr(string name) {
  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {
    PatchObject* obj = ci->second;
    CodeObject* co = obj->co();
    CodeObject::funclist& all = co->funcs();
    for (CodeObject::funclist::iterator fit = all.begin(); fit != all.end(); fit++) {
      if ((*fit)->name().compare(name) == 0) {
        Dyninst::Address addr = (*fit)->addr() + obj->codeBase();
        sp_debug("FOUND - Absolute address of function %s is %lx", name.c_str(), addr);
        return addr;
      }
    }
  }
  sp_debug("NO FOUND - Absolute address of function %s cannot be found", name.c_str());
  return 0;
}

extern sp::SpContext* g_context;
PatchFunction* SpParser::findFunction(string name, bool skip) {
  sp_debug("FIND FUNC - %s", name.c_str());
  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {

    PatchObject* obj = ci->second;
    CodeObject* co = obj->co();
    CodeObject::funclist& all = co->funcs();
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    if (skip && g_context->is_well_known_lib(sp_filename(sym->name().c_str()))) {
      sp_debug("WELL KNOWN - Bypassing well known lib %s", sp_filename(sym->name().c_str()));
      continue;
    }
    sp_debug("IN LIB -  %s", sp_filename(sym->name().c_str()));

    for (CodeObject::funclist::iterator fit = all.begin(); fit != all.end(); fit++) {
      if ((*fit)->name().compare(name) == 0) {
        Dyninst::SymtabAPI::Region* region = sym->findEnclosingRegion((*fit)->addr());
        if (region && region->getRegionName().compare(".plt") == 0) {
          sp_debug("PLT - %s is in plt", name.c_str());
          continue;
        }
        sp_debug("FOUND - %s", name.c_str());
        return obj->getFunc(*fit);
      }
    }
  }
  sp_debug("NOT FOUND - %s", name.c_str());
  return NULL;
}

string SpParser::dump_insn(void* addr, size_t size) {
  using namespace Dyninst::InstructionAPI;
  Dyninst::Address base = (Dyninst::Address)addr;
  SymtabCodeSource* cs = (SymtabCodeSource*)mgr_->as()->executable()->co()->cs();
  string s;
  char buf[256];
  InstructionDecoder deco(addr,
                          size,
                          cs->getArch());
  Instruction::Ptr insn = deco.decode();
  while(insn) {
    sprintf(buf, "    %lx(%2d bytes): %-25s | ", base, insn->size(), insn->format(base).c_str());
    char* raw = (char*)insn->ptr();
    for (int i = 0; i < insn->size(); i++)
      sprintf(buf, "%s%0.2x ", buf, 0xff&raw[i]);
    sprintf(buf, "%s\n", buf);
    s += buf;
    base += insn->size();
    insn = deco.decode();
  }
  return s;
}
