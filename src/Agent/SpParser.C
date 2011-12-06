#include "SpInstrumenter.h"
#include "SpParser.h"
#include "SpContext.h"
#include "SpAddrSpace.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpPointMaker.h"
#include "SpObject.h"

using sp::SpParser;
using sp::SpAddrSpace;
using sb::AddressLookup;
using sb::Symtab;
using sb::Symbol;
using sb::Region;
using pe::CodeObject;
using pe::CodeRegion;
using pe::SymtabCodeSource;
using ph::PatchObject;
using ph::PatchMgrPtr;
using ph::PatchMgr;
using ph::AddrSpace;
using ph::PatchFunction;
using ph::PatchBlock;
using ph::PointMaker;
using ph::Point;
using dt::Address;

extern sp::SpContext* g_context;

SpParser::SpParser()
  : exe_obj_(NULL), injected_(false),sp_offset_(0) {
  init_dyninst_libs();
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
  Address offsets[100];
} IjLib;

/* We will skip dyninst libraries for parsing. */
bool SpParser::is_dyninst_lib(string lib) {
  for (int i = 0; i < dyninst_libs_.size(); i++) {
    if (lib.find(dyninst_libs_[i]) != string::npos) return true;
  }
  return false;
}

void SpParser::init_dyninst_libs() {
  dyninst_libs_.push_back("libpatchAPI.so");
  dyninst_libs_.push_back("libparseAPI.so");
  dyninst_libs_.push_back("libstackwalk.so");
  dyninst_libs_.push_back("libsymtabAPI.so");
  dyninst_libs_.push_back("libinstructionAPI.so");
  dyninst_libs_.push_back("libelf.so");
  dyninst_libs_.push_back("libdwarf.so");
  dyninst_libs_.push_back("libcommon.so");
}

/* The main parsing routine. */
PatchMgrPtr SpParser::parse() {
  if (mgr_) return mgr_;

  /* Get all symtabs in this process */
  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);

  /* Determine whether the agent is injected or preloaded.
     - true: injected by other process (because libijagent.so is loaded ...)
     - false: preloaded
  */
  for (std::vector<Symtab*>::iterator i = tabs.begin(); i != tabs.end(); i++) {
    Symtab* sym = *i;
    if (sym->name().find("libijagent.so") != string::npos) {
      injected_ = true;
      break;
    }
  }

  int shmid;
  key_t key = 1985;
  IjLib* shm;
  if (injected_) {
    if ((shmid = shmget(key, sizeof(IjLib), 0666)) < 0) {
      perror("shmget");
      exit(1);
    }
    if ((char*)(shm = (IjLib*)shmat(shmid, NULL, 0)) == (char *) -1) {
      perror("shmat");
      exit(1);
    }
  }

  /* Build lookup map, to parse those libraries that are loaded before
     injecting this agent.
     This happens only when the agent is injected, rather than preloaded.
  */
  int cur = 0;
  typedef std::map<Address, bool> LibLookup;
  LibLookup lib_lookup;
  lib_lookup[0] = true;
  if (injected_) {
    while (shm->offsets[cur] != -1) {
      lib_lookup[shm->offsets[cur]] = true;
      ++cur;
    }
  }

  PatchObjects patch_objs;
  for (std::vector<Symtab*>::iterator i = tabs.begin(); i != tabs.end(); i++) {
    Symtab* sym = *i;
    Address load_addr = 0;
    al->getLoadAddress(sym, load_addr);

    /* Parsing binary objects is very time consuming
       We avoid parsing the libraries that are not loaded before injecting agent.
    */
    if (injected_) {
      if ((lib_lookup.find(load_addr) == lib_lookup.end())     &&
          (sym->name().find(sp_filename(sp_filename(get_agent_name()))) == string::npos) &&
          (sym->name().find("libagent.so") == string::npos)) {
#ifndef SP_RELEASE
	sp_debug("SKIPED - skip parsing %s", sp_filename(sym->name().c_str()));
#endif
        continue;
      }
    } else {
      if (is_dyninst_lib(sym->name())) {
#ifndef SP_RELEASE
	sp_debug("SKIPED - skip parsing %s", sp_filename(sym->name().c_str()));
#endif
        continue;
      }
    }

    /* Parse binary objects using ParseAPI::CodeObject::parse() */
    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();

    /* Create PatchObjects */
    PatchObject* patch_obj = new sp::SpObject(co, load_addr, NULL, NULL, load_addr?load_addr:scs->loadAddress());

    patch_objs.push_back(patch_obj);
#ifndef SP_RELEASE
    sp_debug("PARSED - parsed %s", sp_filename(sym->name().c_str()));
#endif
    if (sym->isExec()) {
      exe_obj_ = patch_obj;
#ifndef SP_RELEASE
      sp_debug("EXE - %s is an executable", sp_filename(sym->name().c_str()));
#endif
    }
  } // End of symtab iteration
  assert(exe_obj_);

  /* Initialize PatchAPI stuffs */
  AddrSpace* as = SpAddrSpace::create(exe_obj_);
  ph::Instrumenter* inst = NULL;
  inst = sp::SpInstrumenter::create(as);

  sp::SpPointMaker* pm = new SpPointMaker;
  mgr_ = PatchMgr::create(as, inst, pm);
  for (SpParser::PatchObjects::iterator i = patch_objs.begin(); i != patch_objs.end(); i++) {
    if (*i != exe_obj_) {
      as->loadObject(*i);
    }
  }

  /* Destroy shared memory */
  if (injected_) {
    shmctl(IJLIB_ID, IPC_RMID, NULL);
    shmctl(IJMSG_ID, IPC_RMID, NULL);
  }
  return mgr_;
}

/* Find the function that contains addr */
PatchFunction* SpParser::findFunction(Address addr) {

  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {
    PatchObject* obj = ci->second;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    Address lower_bound = obj->codeBase();
    if (!lower_bound) lower_bound = sym->getLoadOffset();
    Address upper_bound = lower_bound + cs->length();

    if (addr >= lower_bound && addr <= upper_bound) {
      Address address = addr;
      sb::Function* f;
      if (!sym->getContainingFunction(address, f)) {
        address -= lower_bound;
      }

      for (std::vector<CodeRegion*>::const_iterator ri = cs->regions().begin();
           ri != cs->regions().end(); ri++) {
        std::set<pe::Function*> funcs;
        obj->co()->findFuncs(*ri, address, funcs);

        if (funcs.size() > 0) {
          PatchFunction* pfunc = obj->getFunc(*funcs.begin());
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

/* TODO: should add support to preloaded mode */
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

/* Get function address from function name. */
Address SpParser::get_func_addr(string name) {
  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {
    PatchObject* obj = ci->second;
    CodeObject* co = obj->co();
    CodeObject::funclist& all = co->funcs();
    for (CodeObject::funclist::iterator fit = all.begin(); fit != all.end(); fit++) {
      if ((*fit)->name().compare(name) == 0) {
        Address addr = (*fit)->addr() + obj->codeBase();
        return addr;
      }
    }
  }
  return 0;
}

/* Find function by name. 
   If `skip` is true, and we can't find the function, then just skip it.
   Otherwise, create the PatchFunction object.
*/
PatchFunction* SpParser::findFunction(string name, bool skip) {
  if (real_func_map_.find(name) != real_func_map_.end()) {
    return real_func_map_[name];
  }

  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {

    PatchObject* obj = ci->second;
    CodeObject* co = obj->co();
    CodeObject::funclist& all = co->funcs();
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    if (skip && g_context->is_well_known_lib(sp_filename(sym->name().c_str()))) {
      continue;
    }

    for (CodeObject::funclist::iterator fit = all.begin(); fit != all.end(); fit++) {
      if ((*fit)->name().compare(name) == 0) {
        Region* region = sym->findEnclosingRegion((*fit)->addr());
        if (region && region->getRegionName().compare(".plt") == 0) {
          continue;
        }
        PatchFunction* found = obj->getFunc(*fit);
        real_func_map_[name] = found;
        return found;
      }
    }
  }
  return NULL;
}

/* Dump instructions in text. */
string SpParser::dump_insn(void* addr, size_t size) {
  using namespace Dyninst::InstructionAPI;
  Address base = (Address)addr;
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

using namespace Dyninst::InstructionAPI;

class SpVisitor : public Visitor {
public:
  SpVisitor(sp::SpPoint* pt)
     : Visitor(), call_addr_(0), pt_(pt), use_pc_(false) { }
  virtual void visit(RegisterAST* r) {
    if (SpParser::is_pc(r->getID())) {
      use_pc_ = true;
      call_addr_ = pt_->block()->end();
    } else {
      /* Non-pc case, x86-32 always goes this way */
      Address rval = pt_->snip()->get_saved_reg(r->getID());
      call_addr_ = rval;
    }
    stack_.push(call_addr_);
  }
  virtual void visit(BinaryFunction* b) {
    Address i1 = stack_.top();
    stack_.pop();
    Address i2 = stack_.top();
    stack_.pop();

    if (b->isAdd()) {
      call_addr_ = i1 + i2;
    } else if (b->isMultiply()) {
      call_addr_ = i1 * i2;
    } else {
      assert(0);
    }

    stack_.push(call_addr_);
  }
  virtual void visit(Immediate* i) {
    Result res = i->eval();
    switch (res.size()) {
    case 1: {
      call_addr_ =res.val.u8val;
      break;
    }
    case 2: {
      call_addr_ =res.val.u16val;
      break;
    }
    case 4: {
      call_addr_ =res.val.u32val;
      break;
    }
    default: {
      call_addr_ =res.val.u64val;
      break;
    }
    }
    stack_.push(call_addr_);
  }
  virtual void visit(Dereference* d) {
    Address* addr = (Address*)stack_.top();
    stack_.pop();
    call_addr_ = *addr;
    stack_.push(call_addr_);
  }

  Address call_addr() const {
    return call_addr_;
  }
  bool use_pc() const {
    return use_pc_;
  }
private:
  std::stack<Address> stack_;
  Address call_addr_;
  sp::SpPoint* pt_;
  bool use_pc_;
};

/* TODO (wenbin): is it okay to cache indirect callee? */
PatchFunction* SpParser::callee(Point* pt, bool parse_indirect) {
  /* 0. Check the cache
     TODO: Should always re-parse indirect call
  */
  sp::SpPoint* spt = static_cast<sp::SpPoint*>(pt);
  if (spt->callee()) return spt->callee();

  /* 1. Looking for direct call */
  PatchFunction* f = pt->getCallee();
  if (f) {
    spt->set_callee(f);
    return f;
  }
  else if (spt->callee()) {
    return spt->callee();
  }

  /* 2. Looking for indirect call */
  if (parse_indirect) {

    PatchBlock* blk = pt->block();
    Instruction::Ptr insn = spt->snip()->get_orig_call_insn();
    Expression::Ptr trg = insn->getControlFlowTarget();
    Address call_addr = 0;
    if (trg) {
      SpVisitor visitor(spt);
      trg->apply(&visitor);
      call_addr = visitor.call_addr();
      f = findFunction(call_addr);
      if (f) {
        spt->set_callee(f);
        return f;
      }
    }
#if 0
  sp_print("CANNOT RESOLVE ADDR %lx, SKIP for blob %lx", call_addr, spt->snip()->buf());

  long *d = (long*)(spt->snip()->get_saved_reg(rr->getID()) + disp);
  sp_print("RR: %s, [%lx+%lx]=>%lx", rr->getID().name().c_str(), spt->snip()->get_saved_reg(rr->getID()), disp, *d);
  sp_print("ORIG CALL DUMP INSN (%d bytes)- {", spt->snip()->get_orig_call_insn()->size());
  sp_print("%s", dump_insn((void*)spt->snip()->get_orig_call_insn()->ptr(), spt->snip()->get_orig_call_insn()->size()).c_str());
  sp_print("DUMP INSN - }");

  sp_print("DUMP INSN (%d bytes)- {", spt->snip()->size());
  sp_print("%s", dump_insn((void*)spt->snip()->buf(), spt->snip()->size()).c_str());
  sp_print("DUMP INSN - }");
#endif
    return NULL;
  }

  return NULL;
}
