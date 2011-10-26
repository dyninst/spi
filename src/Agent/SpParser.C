#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stack>

#include "SpInstrumenter.h"
#include "SpParser.h"
#include "SpCommon.h"
#include "SpContext.h"
#include "SpAddrSpace.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpPointMaker.h"

#include "Point.h"
#include "PatchMgr.h"
#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "CodeObject.h"

#include "Visitor.h"
#include "Immediate.h"
#include "BinaryFunction.h"

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
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::Point;

extern sp::SpContext* g_context;

SpParser::SpParser()
  : exe_obj_(NULL), injected_(false) {
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
  Dyninst::Address offsets[100];
} IjLib;

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

PatchMgrPtr SpParser::parse() {
  if (mgr_) return mgr_;

  // Get all symtabs in this process
  AddressLookup* al = AddressLookup::createAddressLookup(getpid());
  al->refresh();
  std::vector<Symtab*> tabs;
  al->getAllSymtabs(tabs);

  // Determine whether the agent is injected or preloaded.
  // - true: injected by other process
  // - false: preloaded
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

  // Build lookup map, to parse those libraries that are loaded before
  // injecting this agent.
  // This happens only when the agent is injected_, rather than preloaded.
  int cur = 0;
  typedef std::map<Dyninst::Address, bool> LibLookup;
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
    Dyninst::Address load_addr = 0;
    al->getLoadAddress(sym, load_addr);

    // Parsing binary objects is very time consuming
    // We avoid parsing the libraries that are not loaded before injecting agent.
    if (injected_) {
      if ((lib_lookup.find(load_addr) == lib_lookup.end())     &&
          (sym->name().find(sp_filename(sp_filename(get_agent_name()))) == string::npos) &&
          (sym->name().find("libagent.so") == string::npos)) {
        continue;
      }
    } else {
      if (is_dyninst_lib(sym->name())) {
        continue;
      }
    }

    // Parse binary objects using ParseAPI::CodeObject::parse()
    SymtabCodeSource* scs = new SymtabCodeSource(sym);
    code_srcs_.push_back(scs);
    CodeObject* co = new CodeObject(scs);
    code_objs_.push_back(co);
    co->parse();

    // Create PatchObjects
    PatchObject* patch_obj = PatchObject::create(co, load_addr);
    patch_objs.push_back(patch_obj);
    if (sym->isExec()) {
      exe_obj_ = patch_obj;
    }
  } // End of symtab iteration
  assert(exe_obj_);

  // Initialize PatchAPI stuffs
  AddrSpace* as = SpAddrSpace::create(exe_obj_);
  Dyninst::PatchAPI::Instrumenter* inst = NULL;
  inst = sp::JumpInstrumenter::create(as);

  sp::SpPointMaker* pm = new SpPointMaker;
  mgr_ = PatchMgr::create(as, inst, pm);
  for (SpParser::PatchObjects::iterator i = patch_objs.begin(); i != patch_objs.end(); i++) {
    if (*i != exe_obj_) {
      as->loadObject(*i);
    }
  }

  // destroy shared memory
  if (injected_) {
    shmctl(IJLIB_ID, IPC_RMID, NULL);
    shmctl(IJMSG_ID, IPC_RMID, NULL);
  }
  return mgr_;
}

/* Find the function that contains addr */
PatchFunction* SpParser::findFunction(Dyninst::Address addr) {
  //  assert(0);
  AddrSpace* as = mgr_->as();
  for (AddrSpace::ObjMap::iterator ci = as->objMap().begin(); ci != as->objMap().end(); ci++) {
    PatchObject* obj = ci->second;
    SymtabCodeSource* cs = (SymtabCodeSource*)obj->co()->cs();
    Symtab* sym = cs->getSymtabObject();
    Dyninst::Address lower_bound = obj->codeBase();
    if (!lower_bound) lower_bound = sym->getLoadOffset();
    Dyninst::Address upper_bound = lower_bound + cs->length();

    if (addr >= lower_bound && addr <= upper_bound) {
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
        return addr;
      }
    }
  }
  return 0;
}

PatchFunction* SpParser::findFunction(string name, bool skip) {
  // sp::findfunc_start();

  if (real_func_map_.find(name) != real_func_map_.end()) {
    // sp::findfunc_end();
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
        Dyninst::SymtabAPI::Region* region = sym->findEnclosingRegion((*fit)->addr());
        if (region && region->getRegionName().compare(".plt") == 0) {
          continue;
        }
        PatchFunction* found = obj->getFunc(*fit);
        real_func_map_[name] = found;
        //sp::findfunc_end();
        return found;
      }
    }
  }
  //sp::findfunc_end();
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

using namespace Dyninst::InstructionAPI;
class SpVisitor : public Visitor {
public:
  SpVisitor(SpParser* p, Point* pt)
     : Visitor(), p_(p), call_addr_(0), pt_(pt), use_pc_(false) { }
  virtual void visit(RegisterAST* r) {

    if (p_->is_pc(r->getID())) {
      use_pc_ = true;
      call_addr_ = pt_->block()->end();
    } else {
      sp::SpPoint* spt = static_cast<sp::SpPoint*>(pt_);
      Dyninst::Address rval = p_->get_saved_reg(r->getID(), *spt->saved_context_ptr(),
                                                pt_->block()->end() - pt_->block()->last());
      call_addr_ = rval;
    }
    stack_.push(call_addr_);
  }
  virtual void visit(BinaryFunction* b) {
    Dyninst::Address i1 = stack_.top();
    stack_.pop();
    Dyninst::Address i2 = stack_.top();
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
    Dyninst::Address* addr = (Dyninst::Address*)stack_.top();
    stack_.pop();
    call_addr_ = *addr;
    stack_.push(call_addr_);
  }

  Dyninst::Address call_addr() const {
    return call_addr_;
  }
  bool use_pc() const {
    return use_pc_;
  }
private:
  SpParser* p_;
  std::stack<Dyninst::Address> stack_;
  Dyninst::Address call_addr_;
  Point* pt_;
  bool use_pc_;
};

PatchFunction* SpParser::callee(Point* pt, bool parse_indirect) {
  //-------------------------------------
  // 0. Check the cache
  //-------------------------------------
  sp::SpPoint* spt = static_cast<sp::SpPoint*>(pt);
  if (spt->callee()) return spt->callee();

  //-------------------------------------
  // 1. Looking for direct call
  //-------------------------------------
  PatchFunction* f = pt->getCallee();
  if (f) {
    //PatchFunction* real_func = findFunction(f->name());
    //if (real_func) f = real_func;
    spt->set_callee(f);
    return f;
  }

  //-------------------------------------
  // 2. Looking for indirect call
  //-------------------------------------
  if (parse_indirect) {
    sp_debug("PARSE INDIRECT CALL");
    PatchBlock* blk = pt->block();
    Instruction::Ptr insn = blk->getInsn(blk->last());
    Expression::Ptr trg = insn->getControlFlowTarget();
    SpVisitor visitor(this, pt);
    trg->apply(&visitor);

    Dyninst::Address call_addr = visitor.call_addr();

    sp_debug("INDIRECT - to %lx", call_addr);
    f = findFunction(call_addr);
    if (f) {
      spt->set_callee(f);
/*
      if (visitor.use_pc()) {
        using namespace Dyninst::PatchAPI;
        SpContext::InstMap& inst_map = g_context->inst_map();
        if (inst_map.find(pt->block()->last()) == inst_map.end()) return f;
        InstancePtr instance = inst_map[pt->block()->last()];
        Snippet<SpSnippet::ptr>::Ptr snip = Snippet<SpSnippet::ptr>::get(instance->snippet());
        SpSnippet::ptr sp_snip = snip->rep();
        sp_snip->fixup(f);
      }
      sp::callee_end();
*/
      return f;
    }

    sp_print("CANNOT RESOLVE ADDR %lx, SKIP", call_addr);
    return NULL;
  }

  return NULL;
}
