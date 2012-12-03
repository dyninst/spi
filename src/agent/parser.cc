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

#include <dirent.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <stack>

#include "instructionAPI/h/BinaryFunction.h"
#include "instructionAPI/h/Immediate.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/Visitor.h"
#include "patchAPI/h/PatchMgr.h"
#include "symtabAPI/h/AddrLookup.h"
#include "symtabAPI/h/Symtab.h"

#include "agent/context.h"
#include "agent/parser.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/instrumenter.h"
#include "agent/patchapi/maker.h"
#include "agent/patchapi/object.h"
#include "agent/patchapi/point.h"
#include "common/utils.h"
#include "injector/injector.h"



namespace sp {

extern SpContext* g_context;
extern SpParser::ptr g_parser;

SpParser::SpParser()
    : injected_(false), exe_obj_(NULL) {

  binaries_to_inst_.insert(sp_filename(GetExeName().c_str()));
  binaries_to_inst_.insert("libagent.so");
}

// Clean up memory buffers for ParseAPI stuffs
SpParser::~SpParser() {
  for (CodeSources::iterator i = code_srcs_.begin();
       i != code_srcs_.end(); i++) {
    delete static_cast<pe::SymtabCodeSource*>(*i);
  }
  for (CodeObjects::iterator i = code_objs_.begin();
       i != code_objs_.end(); i++) {
    delete *i;
  }

  // Destory shared memory we use
  shmctl(IJMSG_ID, IPC_RMID, NULL);
}

// Agent-writer can create their own Parser. So use shared point to make
// memory management easy
SpParser::ptr
SpParser::Create() {
  return ptr(new SpParser);
}

// ------------------------------------------------------------------- 
// Parsing the binary
// -------------------------------------------------------------------

// Get symtabs during runtime
// Side effect:
// 1. determine whether this agent is injected or preloaded
// 2. figure out the agent library's name

sb::AddressLookup*
SpParser::GetRuntimeSymtabs(sp::SymtabSet& symtabs) {

  // Get all symtabs in this process
  
  sb::AddressLookup* al =
      sb::AddressLookup::createAddressLookup(getpid());
  if (!al) {
    sp_debug("FAILED TO CREATE AddressLookup");
    return NULL;
  }
  al->refresh();
  std::vector<sb::Symtab*> tabs;
  al->getAllSymtabs(tabs);
  if (tabs.size() == 0) {
    sp_debug("FOUND NO SYMTABS");
    return NULL;
  }
  sp_debug("SYMTABS - %ld symtabs found", (long)tabs.size());

  for (std::vector<sb::Symtab*>::iterator i = tabs.begin();
       i != tabs.end(); i++) {
    sb::Symtab* sym = *i;

    // Figure the name for agent library
    // We rely on looking for the magic variable -
    // " int self_propelled_instrumentation_agent_library = 19861985; "
    
    string magic_var = "self_propelled_instrumentation_agent_library";
    Symbols syms;
    if (sym->findSymbol(syms, magic_var) && syms.size() > 0) {
      agent_name_ = sym->name();
      sp_debug("AGENT NAME - %s", agent_name_.c_str());
    }

    // Deduplicate symtabs
    
    symtabs.insert(sym);

    // Determine whether the agent is injected or preloaded. If injected_ is:
    //   - true: injected by other process (libijagent.so is loaded)
    //   - false: preloaded
    
    if (sym->name().find("libijagent.so") != string::npos) {
      injected_ = true;
    }
  }
  return al;
}

// Create a bunch of PatchObjects from Symtabs
bool
SpParser::CreatePatchobjs(sp::SymtabSet& unique_tabs,
                          sb::AddressLookup* al,
                          sp::PatchObjects& patch_objs) {
  // The main loop to build PatchObject for all dependencies
  
  for (SymtabSet::iterator i = unique_tabs.begin();
       i != unique_tabs.end(); i++) {
    sb::Symtab* sym = *i;
    dt::Address load_addr = 0;
    al->getLoadAddress(sym, load_addr);

    // Parsing binary objects is very time consuming. We avoid
    // parsing the libraries that are either well known (e.g., libc,
    // or dyninst).
    string libname_no_path = sp_filename(sym->name().c_str());

    if (!CanInstrumentLib(libname_no_path)) {
      sp_debug("SKIPED - skip parsing %s",
               sp_filename(sym->name().c_str()));
      continue;
    }

    SpObject* patch_obj = CreateObject(sym, load_addr);
    if (!patch_obj) {
      sp_debug("FAILED TO CREATE PATCH OBJECT");
      continue;
    }

    patch_objs.push_back(patch_obj);
    sp_debug("PARSED - parsed %s", sp_filename(sym->name().c_str()));

    if (sym->isExec()) {
      exe_obj_ = patch_obj;
      sp_debug("EXE - %s is an executable",
               sp_filename(sym->name().c_str()));
    }
  } // End of symtab iteration

  return true;
}

// Get mutatee's executable's SpObject
SpObject*
SpParser::GetExeFromProcfs(sp::PatchObjects& patch_objs) {
  for (PatchObjects::iterator oi = patch_objs.begin();
       oi != patch_objs.end(); oi++) {
    SpObject* obj = OBJ_CAST(*oi);
    assert(obj);
    char* s1 = sp_filename(sp::GetExeName().c_str());
    char* s2 = sp_filename(obj->name().c_str());
    // sp_debug("PARSE EXE - s1=%s, s2 =%s", s1, s2);
    if (strcmp(s1, s2) == 0) {
      sp_debug("GOT EXE - %s is an executable shared library", s2);
      return obj;
    } // strcmp
  } // for each obj
  return NULL;
}

// Create a PatchMgr object. Side effects:
// - Initialize memory allocator, which preallocates small buffers
ph::PatchMgrPtr
SpParser::CreateMgr(sp::PatchObjects& patch_objs) {
  SpAddrSpace* as = SpAddrSpace::Create(exe_obj_);
  assert(as);

  SpInstrumenter* inst = sp::SpInstrumenter::create(as);
  assert(inst);

  ph::PatchMgrPtr mgr = ph::PatchMgr::create(as,
                                             inst,
                                             new SpPointMaker);
  assert(mgr);

  // Load each parsed objects into address space
  for (PatchObjects::iterator i = patch_objs.begin();
       i != patch_objs.end(); i++) {
    if (*i != exe_obj_) {
      as->loadObject(*i);
    }
  }

  // Initialize memory allocator
  as->InitMemoryAllocator();
  
  return mgr;
}

SpObject*
SpParser::CreateObject(sb::Symtab* sym,
                       dt::Address load_addr) {
  assert(sym);
  SpObject* obj = NULL;
  bool parse_from_file = false;
  char* no_path_name = sp_filename(sym->name().c_str());
  if (getenv("SP_PARSE_DIR")) {
    // Do name matching. Here we assume only one file hierarchy in
    // this directory $SP_PARSE_DIR
    DIR* dir = opendir(getenv("SP_PARSE_DIR"));
    if (dir) {
      dirent* entry = readdir(dir);
      while (entry) {
        sp_debug("Parsed library: %s is %s?", entry->d_name,
                 no_path_name);
        if (strcmp(no_path_name, entry->d_name) == 0) {
          sp_debug("GOT Parsed LIB: %s", entry->d_name);
          parse_from_file = true;
          break;
        }
        entry = readdir(dir);
      }
      closedir(dir);
    } else {
      sp_debug("FAILED TO ITERATE DIR %s", getenv("SP_PARSE_DIR"));
    }
  }

  if (parse_from_file) {
    sp_debug("PARSE FROM FILE - for %s", sym->name().c_str());
    obj = CreateObjectFromFile(sym, load_addr);
  } else {
    sp_debug("PARSE FROM RUNTIME - for %s", sym->name().c_str());
    obj = CreateObjectFromRuntime(sym, load_addr);
  }
  return obj;
}

SpObject*
SpParser::CreateObjectFromRuntime(sb::Symtab* sym,
                                  dt::Address load_addr) {

  // Parse binary objects using ParseAPI::CodeObject::parse().
  pe::SymtabCodeSource* scs = new pe::SymtabCodeSource(sym);
  code_srcs_.push_back(scs);
  pe::CodeObject* co = new pe::CodeObject(scs);
  code_objs_.push_back(co);
  co->parse();

  // Uncomment this to parse stripped object
  /*
    vector<CodeRegion *>::const_iterator rit = scs->regions().begin();
    for( ; rit != scs->regions().end(); ++rit) {
    SymtabCodeRegion * scr = static_cast<SymtabCodeRegion*>(*rit);
    if(scr->symRegion()->isText()) {
    co->parseGaps(scr);
    }
    }
  */

  dt::Address real_load_addr =
      load_addr ? load_addr : scs->loadAddress();
  // sp_print("load_addr: %lx, scs->loadAddress: %lx", load_addr,
  //         scs->loadAddress());
  SpObject* obj =  new sp::SpObject(co,
                                    load_addr,
                                    new SpCFGMaker,
                                    NULL,
                                    real_load_addr,
                                    sym);

  // TODO: If needed, we create a serialized object
  return obj;
}

SpObject*
SpParser::CreateObjectFromFile(sb::Symtab* sym,
                               dt::Address load_addr) {
  SpObject* obj = NULL;
  assert(0 && "TODO");
  return obj;
}

// The main parsing routine.
// This is the default implementation, which parses binary during
// runtime.
// Agent-writer can provide their own parse() method.
ph::PatchMgrPtr
SpParser::Parse() {
  // Avoid duplicate parsing. If a shared library is loaded during
  // runtime, we have to use PatchAPI's loadLibrary method to add
  // this shared library
  if (mgr_) return mgr_;

  // Step 1: Parse runtime symtabs
  SymtabSet unique_tabs;
  sb::AddressLookup* al = GetRuntimeSymtabs(unique_tabs);
  if (!al) {
    sp_perror("FAILED TO GET RUNTIME SYMTABS");
  }
  if (agent_name_.size() == 0) {
    agent_name_ = "libagent.so";
  }
  binaries_to_inst_.insert(agent_name_);

  // Step 2: Create patchapi objects
  PatchObjects patch_objs;
  if (!CreatePatchobjs(unique_tabs, al, patch_objs)) {
    sp_perror("FAILED TO CREATE PATCHOBJS");
  }

  // Step 3: In the case of executable shared library, we cannot get
  // exe_obj_ via symtabAPI, so we resort to /proc
  if (!exe_obj_) {
    exe_obj_ = GetExeFromProcfs(patch_objs);
    if (!exe_obj_) {
      sp_perror("FAILED TO GET EXE OBJ FROM PROC FS");
    }
  } // !exe_obj_

  // Step 4: Initialize PatchAPI stuffs
  mgr_ = CreateMgr(patch_objs);
  if (!mgr_) {
    sp_perror("FAILED TO CREATE PATCHMGR");
  }

  return mgr_;
}



// Dump instructions in text.
string
SpParser::DumpInsns(void* addr,
                    size_t size) {

  dt::Address base = (dt::Address)addr;
  pe::CodeSource* cs = exe_obj_->co()->cs();
  string s;
  char buf[256];
  in::InstructionDecoder deco(addr, size, cs->getArch());
  in::Instruction::Ptr insn = deco.decode();
  while(insn) {
    sprintf(buf, "    %lx(%2lu bytes): %-25s | ", base,
            (unsigned long)insn->size(), insn->format(base).c_str());
    char* raw = (char*)insn->ptr();
    for (unsigned i = 0; i < insn->size(); i++)
      sprintf(buf, "%s%2x ", buf, 0xff&raw[i]);
    sprintf(buf, "%s\n", buf);
    s += buf;
    base += insn->size();
    insn = deco.decode();
  }
  return s;
}

// To calculate absolute address for indirect function call
class SpVisitor : public in::Visitor {
 public:
  SpVisitor(sp::SpPoint* pt,
            dt::Address seg_val)
      : Visitor(),
        call_addr_(0),
        pt_(pt),
        seg_val_(seg_val) { }
  virtual void visit(in::RegisterAST* r) {
    if (r->getID().isPC()) {
      call_addr_ = pt_->block()->end();
      // call_addr_ = pt_->block()->last();
    } else {
      // Non-pc case, x86-32 always goes this way
      dt::Address rval = pt_->snip()->GetSavedReg(r->getID());
      sp_debug("GOT NON-PC REG - %s = %lx",
               r->getID().name().c_str(), rval);
      call_addr_ = rval;
    }

    sp_debug("SP_VISITOR - reg value %lx", call_addr_);
    stack_.push(call_addr_);
  }
  virtual void visit(in::BinaryFunction* b) {
    dt::Address i1 = stack_.top();
    stack_.pop();
    dt::Address i2 = stack_.top();
    stack_.pop();

    if (b->isAdd()) {
      call_addr_ = i1 + i2;
      sp_debug("SP_VISITOR - %lx + %lx = %lx", i1, i2, call_addr_);
    } else if (b->isMultiply()) {
      call_addr_ = i1 * i2;
      sp_debug("SP_VISITOR - %lx * %lx = %lx", i1, i2, call_addr_);
    } else {
      assert(0);
    }

    stack_.push(call_addr_);
  }
  virtual void visit(in::Immediate* i) {
    in::Result res = i->eval();
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

    sp_debug("SP_VISITOR - imm %lx ", call_addr_);
    stack_.push(call_addr_);
  }
  virtual void visit(in::Dereference* d) {
    dt::Address* addr = (dt::Address*)(stack_.top() + seg_val_);
    stack_.pop();
    sp_debug("SP_VISITOR - dereferencing %lx => ? ",
             (dt::Address)addr);
    call_addr_ = *addr;
    sp_debug("SP_VISITOR - dereference %lx => %lx ",
             (dt::Address)addr, call_addr_);
    stack_.push(call_addr_);
  }

  dt::Address call_addr() const {
    return call_addr_;
  }

 private:
  std::stack<dt::Address> stack_;
  dt::Address call_addr_;
  sp::SpPoint* pt_;
  dt::Address seg_val_;
};

// TODO (wenbin): is it okay to cache indirect callee?
SpFunction*
SpParser::callee(SpPoint* pt,
                 bool parse_indirect) {
  assert(pt);

  // 0. Check the cache
  SpFunction* f = pt->callee();

  if (f) {
    return f;
  }

  // 1. Looking for direct call
  f = FUNC_CAST(pt->getCallee());
  if (f) {
    SpFunction* tmp_f = g_parser->FindFunction(f->GetMangledName());
    if (tmp_f && tmp_f != f) {
      f = tmp_f;
    }

    SpFunction* sfunc = f;
    assert(sfunc);
    pt->SetCallee(sfunc);
    return sfunc;
  }
  
  // 2. Looking for indirect call
  if (parse_indirect) {
    
    SpBlock* b = pt->GetBlock();
    assert(b);

    if (addr_callee_not_found_.find(b->last()) !=
                                    addr_callee_not_found_.end()) {
      sp_debug("NOT FOUND - proved not found for %lx", b->last());
      return NULL;
    }

    sp_debug("PARSING INDIRECT - for call insn %lx",
             b->last());

    in::Instruction::Ptr insn = b->orig_call_insn();
    assert(insn);

    sp_debug("DUMP INDCALL INSN (%ld bytes)- {", (long)insn->size());
    sp_debug("%s",
             DumpInsns((void*)insn->ptr(),
                       insn->size()).c_str());
    sp_debug("DUMP INSN - }");

    in::Expression::Ptr trg = insn->getControlFlowTarget();
    dt::Address call_addr = 0;
    if (trg) {
      dt::Address segment_reg_val = 0;

      SpVisitor visitor(pt, segment_reg_val);
      trg->apply(&visitor);
      call_addr = visitor.call_addr();
      
      sp_debug("GOT CALL_ADDR - %lx", call_addr);
      f = FindFunction(call_addr);
      if (f) {
        SpFunction* sfunc = FUNC_CAST(f);
        assert(sfunc);
        sp_debug("PARSED INDIRECT - %lx is %s in %s", b->last(),
                 sfunc->name().c_str(),
                 sfunc->GetObject()->name().c_str());
        pt->SetCallee(sfunc);
        return sfunc;
      }
    }

    sp_debug("CANNOT FIND INDRECT CALL - for call insn %lx",
             b->last());
    addr_callee_not_found_.insert(b->last());
    return NULL;
  }
  
  return NULL;
}

// ------------------------------------------------------------------- 
// Cooperating with Injector
// -------------------------------------------------------------------

// The structure that stores information passed by Injector
typedef struct {
  char libname[512];   // Agent library name
  char err[512];       // Error message
  char loaded;         // Whether the library is loaded successfully
  long pc;             // The pc reg value when the mutatee is interrupted
  long sp;             // The sp reg value when the mutatee is interrupted
  long bp;             // The bp reg value when the mutatee is interrupted
} IjMsg;

// Get pc / sp / bp reg values from IjMsg structure stored in shared mem
void
SpParser::GetFrame(long* pc,
                   long* sp,
                   long* bp) {
  IjMsg* shm = (IjMsg*)GetSharedMemory(1986, sizeof(IjMsg));
  *pc = shm->pc;
  *sp = shm->sp;
  *bp = shm->bp;
}



void
SpParser::SetLibrariesToInstrument(const StringSet& libs) {
  for (StringSet::iterator i = libs.begin(); i != libs.end(); i++)
    binaries_to_inst_.insert(*i);
}

void
SpParser::SetFuncsNotToInstrument(const StringSet& funcs) {
  for (StringSet::iterator i = funcs.begin(); i != funcs.end(); i++)
    funcs_not_to_inst_.insert(*i);
}

bool
SpParser::CanInstrumentLib(const std::string& lib) {
  for (StringSet::iterator i = binaries_to_inst_.begin();
       i != binaries_to_inst_.end(); i++) {
    if (lib.find(*i) != string::npos) return true;
  }
  return false;
}

bool
SpParser::CanInstrumentFunc(const std::string& func) {
  for (StringSet::iterator i = funcs_not_to_inst_.begin();
       i != funcs_not_to_inst_.end(); i++) {
    if (func.find(*i) != string::npos) return false;
  }
  return true;
}

// ------------------------------------------------------------------- 
// Get functions by name or by address
// -------------------------------------------------------------------

// Get function address from function name.

dt::Address
SpParser::GetFuncAddrFromName(string name) {
  
  FuncSet found_funcs;
  // We expect users to provide pretty name
  if (!FindFunction(name, &found_funcs) || found_funcs.size() == 0) {
    sp_debug("GET FUNC ADDR FROM NAME - failed for %s", name.c_str());
    return 0;
  }

  // To simplify things, We don't want the payload function to be overloaded
  if (found_funcs.size() > 1) {
    sp_debug("GET FUNC ADDR FROM NAME - failed for %s, overloaded func",
             name.c_str());
    return 0;
  }
  
  return (*found_funcs.begin())->addr();
}

// Find the function that contains absolute addr

SpFunction*
SpParser::FindFunction(dt::Address addr) {
  sp_debug("FIND FUNC BY ADDR - for call insn %lx", addr);

  if (addr == 0) {
    return NULL;
  }
  
  // A quick return, if this function is in the cache
  if (addr_func_map_.find(addr) != addr_func_map_.end()) {
    sp_debug("GOT FROM CACHE - %s",
             addr_func_map_[addr]->name().c_str());
    return addr_func_map_[addr];
  }

  // A quick return, if this function is proved to be not found
  if (addr_func_not_found_.find(addr) != addr_func_not_found_.end()) {
    sp_debug("NOT FOUND - call at %lx is proved to be not found", addr);
    return NULL;
  }
  
  ph::AddrSpace* as = mgr_->as();
  for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
       ci != as->objMap().end(); ci++) {
    SpObject* obj = OBJ_CAST(ci->second);

    // Get the function offset relative to the object
    dt::Address offset = addr - obj->codeBase();
    
    pe::CodeObject* co = obj->co();
    pe::CodeSource* cs = co->cs();

    set<pe::CodeRegion *> match;
    set<pe::Block *> blocks;
    int cnt = cs->findRegions(offset, match);

    if(cnt != 1)
      continue;
    else if(cnt == 1) {
      co->findBlocks(*match.begin(), offset, blocks);

      sp_debug("%ld blocks found", (long)blocks.size());
      if (blocks.size() == 1) {
        std::vector<pe::Function*> funcs;
        (*blocks.begin())->getFuncs(funcs);
        SpFunction* found = FUNC_CAST(obj->getFunc(*funcs.begin()));
        if (found) {
          addr_func_map_[addr] = found;
          return found;
        }
      }
      addr_func_not_found_.insert(addr);
      return NULL;
    }
  }
  addr_func_not_found_.insert(addr);
  return NULL;
}

// Find functions by pretty name

bool
SpParser::FindFunction(string name,
                       FuncSet* found_funcs) {
  sp_debug("LOOKING FOR FUNC BY PRETTY NAME - looking for %s", name.c_str());

  if (name.length() == 0) {
    return false;
  }

  // A quick return, if this function is in the cache
  if (demangled_func_map_.find(name) != demangled_func_map_.end()) {
    sp_debug("GOT FROM CACHE - %s", name.c_str());

    std::copy(demangled_func_map_[name].begin(),
              demangled_func_map_[name].end(),
              inserter(*found_funcs, found_funcs->begin()));
    return true;
  }

  // A quick return, if this function is proved to be not found
  if (demangled_func_not_found_.find(name)
      != demangled_func_not_found_.end()) {
    sp_debug("NOT FOUND - %s is proved to be not found", name.c_str());
    return false;
  }
  
  // Iterate through each object to look for this function
  assert(mgr_);
  ph::AddrSpace* as = mgr_->as();
  FuncSet func_set;
  for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
       ci != as->objMap().end(); ci++) {
    SpObject* obj = OBJ_CAST(ci->second);

    GetFuncsByName(obj, name, false, &func_set);
  }

  if (func_set.size() == 0) {
    demangled_func_not_found_.insert(name);
    return false;
  }

  // Add to cache
  std::copy(func_set.begin(), func_set.end(),
            inserter(demangled_func_map_[name],
                     demangled_func_map_[name].begin()));
  
  std::copy(func_set.begin(), func_set.end(),
            inserter(*found_funcs, found_funcs->begin()));
  assert(found_funcs->size() > 0);
  return true;
}

// Find function by name.

SpFunction*
SpParser::FindFunction(string name) {

  sp_debug("LOOKING FOR FUNC BY MANGLED NAME - looking for %s",
           name.c_str());
  
  if (name.length() == 0) {
    return NULL;
  }

  // A quick return, if this function is in the cache
  if (mangled_func_map_.find(name) != mangled_func_map_.end()) {
    sp_debug("GOT FROM CACHE - %s",
             mangled_func_map_[name]->name().c_str());
    return mangled_func_map_[name];
  }

  // A quick return, if this function is proved to be not found
  if (mangled_func_not_found_.find(name) != mangled_func_not_found_.end()) {
    sp_debug("NOT FOUND - %s is proved to be not found", name.c_str());
    return NULL;
  }
  
  // Iterate through each object to look for this function
  assert(mgr_);
  ph::AddrSpace* as = mgr_->as();
  FuncSet func_set;
  for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
       ci != as->objMap().end(); ci++) {
    SpObject* obj = OBJ_CAST(ci->second);

    if (strcmp(sp_filename(obj->name().c_str()), "libagent.so") == 0) {
      sp_debug("SKIP - lib %s", sp_filename(obj->name().c_str()));
      continue;
    }
    
    GetFuncsByName(obj, name, true, &func_set);
  }

  // We skip the case multiple functions have the same name
  if (func_set.size() == 1) {
    mangled_func_map_[name] = *func_set.begin();
    sp_debug("FOUND - %s in object %s", name.c_str(),
             FUNC_CAST((*func_set.begin()))->GetObject()->name().c_str());
    return *func_set.begin();
  }

  mangled_func_not_found_.insert(name);  
  sp_debug("NO FOUND - %s", name.c_str());
  return NULL;
}

size_t total_count=0;
size_t total_size=0;
// Internal function used by FindFunction
bool
SpParser::GetFuncsByName(sp::SpObject* obj,
                         std::string name,
                         bool mangled,
                         sp::FuncSet* func_set) {
  assert(obj);
  assert(func_set);
  sp_debug("IN OBJECT - %s in %s?",
           name.c_str(), obj->name().c_str());

  // Two cases to skip:
  // 1. Not in inst_lib list
  // 2. In inst_lib, lib is libc, but function is not __libc_start_main
  if (!CanInstrumentLib(sp_filename(obj->name().c_str()))) {
    sp_debug("SKIP - lib %s", sp_filename(obj->name().c_str()));
    return false;
  }

  if (obj->name().find("libc-") != std::string::npos &&
      strcmp(name.c_str(), "__libc_start_main") != 0) {
    sp_debug("SKIP - lib %s and non __libc_start_main",
             sp_filename(obj->name().c_str()));
    return false;
  }
  
  sb::Symtab* sym = obj->symtab();
  assert(sym);

  pe::CodeObject* co = obj->co();
  assert(co);
  
  const pe::CodeObject::funclist& all = co->funcs();
  for (pe::CodeObject::funclist::iterator fit = all.begin();
       fit != all.end(); fit++) {

    // sp_debug("FUNC - %s", (*fit)->name().c_str());
    // Get or create a PatchFunction instance

    SpFunction* found = FUNC_CAST(obj->getFunc(*fit));
    total_count++;
    total_size += sizeof(SpFunction);
    
    // Skip unmatched function

    if (mangled) {
      if (!found ||
          found->GetMangledName().length() == 0 ||
          found->GetMangledName().compare(name) != 0) {
        /*
          sp_debug("SKIP %s - mangled name %s length %ld",
          (*fit)->name().c_str(),
          found->GetMangledName().c_str(),
          found->GetMangledName().length());
        */
        continue;
      }
    } else {
      if (!found ||
          found->name().length() == 0 ||
          found->name().compare(name) != 0) {
        continue;
      }      
    }
    
    // Skip .plt functions
    sb::Region* region = sym->findEnclosingRegion((*fit)->addr());
    assert(region);
    if (region->getRegionName().compare(".plt") == 0) {
      sp_debug("A PLT, SKIP - %s at %lx", name.c_str(),
               (*fit)->addr() - obj->load_addr());
      continue;
    }

    
    sp_debug("GOT %s in OBJECT - %s at %lx",
             name.c_str(), sym->name().c_str(),
             found->addr() - obj->load_addr());

    if (found) {
      func_set->insert(found);
    }
  } // For each function

  return true;
}

//////////////////////////////////////////////////////////////////////
// Parse library that is instrumentable and not parsed yet
// TODO: Should hook up memory allocator
bool
SpParser::ParseDlExit(SpPoint* pt) {

  ph::PatchFunction* f = sp::Callee(pt);
  if (f && f->name().compare("dlopen") != 0) return true;

  SymtabSet unique_tabs;
  sb::AddressLookup* al = g_parser->GetRuntimeSymtabs(unique_tabs);
  if (!al) {
    sp_perror("FAILED TO GET RUNTIME SYMTABS");
  }

  ph::AddrSpace* as = g_parser->mgr()->as();
  assert(as);
  StringSet orig_obj_names;
  for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
       ci != as->objMap().end(); ci++) {
    sp::SpObject* obj = OBJ_CAST(ci->second);
    // sp_print("%s", obj->name().c_str());
    orig_obj_names.insert(obj->name());
  }

  for (SymtabSet::iterator si = unique_tabs.begin();
       si != unique_tabs.end(); si++) {
    sb::Symtab* sym = *si;
    // sp_print("handling %s", sym->name().c_str());
    if (!g_parser->CanInstrumentLib(sym->name().c_str())) {
      // sp_print("Cannot instrument %s", sym->name().c_str());
      continue;
    } else if (orig_obj_names.find(sym->name()) == orig_obj_names.end()) {
      // sp_print("Newly added library: %s", sym->name().c_str());
      dt::Address load_addr = 0;
      al->getLoadAddress(sym, load_addr);
      SpObject* patch_obj = g_parser->CreateObject(sym, load_addr);
      if (!patch_obj) {
        sp_debug("FAILED TO CREATE PATCH OBJECT");
        return false;
      }
      as->loadObject(patch_obj);
      return true;
    }
  }

  return true;
}

}
