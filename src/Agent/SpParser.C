#include "SpMaker.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpParser.h"
#include "SpObject.h"
#include "SpContext.h"
#include "SpInjector.h"
#include "SpAddrSpace.h"
#include "SpInstrumenter.h"

// Magic number to identify agent library
// ( make the name long enough ... )
int self_propelled_instrumentation_agent_library = 19861985;

namespace sp {

  extern SpContext* g_context;
	extern SpParser::ptr g_parser;

  SpParser::SpParser()
    : injected_(false), exe_obj_(NULL) {

    init_well_known_libs();
    init_dyninst_libs();

  }

  // Clean up memory buffers for ParseAPI stuffs
  SpParser::~SpParser() {
    for (CodeSources::iterator i = code_srcs_.begin();
         i != code_srcs_.end(); i++) {
			pe::SymtabCodeSource* scs = static_cast<pe::SymtabCodeSource*>(*i);
      delete scs;
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
  SpParser::create() {
    return ptr(new SpParser);
  }

  // We skip "well known libraries" to parse
  void
  SpParser::init_well_known_libs() {
    well_known_libs_.insert("libc-");
    well_known_libs_.insert("libm-");
    well_known_libs_.insert("ld-");
    well_known_libs_.insert("libdl-");
    well_known_libs_.insert("libstdc++");
    well_known_libs_.insert("libgcc");
    well_known_libs_.insert("libpthread-");
  }

  bool
  SpParser::is_well_known_lib(string lib) {
    for (StringSet::iterator i = well_known_libs_.begin();
         i != well_known_libs_.end(); i++) {
      if (lib.find(*i) != string::npos) return true;
    }
    return false;
  }

  // We will skip dyninst libraries for parsing.
  bool
  SpParser::is_dyninst_lib(string lib) {
    for (StringSet::iterator i = dyninst_libs_.begin();
         i != dyninst_libs_.end(); i++) {
      if (lib.find(*i) != string::npos) return true;
    }
    return false;
  }

  void
  SpParser::init_dyninst_libs() {
    dyninst_libs_.insert("libpatchAPI.so");
    dyninst_libs_.insert("libparseAPI.so");
    dyninst_libs_.insert("libstackwalk.so");
    dyninst_libs_.insert("libsymtabAPI.so");
    dyninst_libs_.insert("libinstructionAPI.so");
    dyninst_libs_.insert("libelf.so");
    dyninst_libs_.insert("libdwarf.so");
    dyninst_libs_.insert("libcommon.so");
    dyninst_libs_.insert("libpcontrol.so");
  }

  // The main parsing routine.
  // This is the default implementation, which parses binary during runtime.
  // Agent-writer can provide their own parse() method.
	ph::PatchMgrPtr
  SpParser::parse() {

    // Avoid duplicate parsing. If a shared library is loaded during runtime,
    // we have to use PatchAPI's loadLibrary method to add this shared library
    if (mgr_) return mgr_;

    // Get all symtabs in this process
		sb::AddressLookup* al = sb::AddressLookup::createAddressLookup(getpid());
    al->refresh();
    std::vector<sb::Symtab*> tabs;
    al->getAllSymtabs(tabs);

    // Determine whether the agent is injected or preloaded.
    //   - true: injected by other process (because libijagent.so is loaded)
    //   - false: preloaded
    for (std::vector<sb::Symtab*>::iterator i = tabs.begin();
         i != tabs.end(); i++) {
			sb::Symtab* sym = *i;
      if (sym->name().find("libijagent.so") != string::npos) {
        injected_ = true;
        break;
      }
    }

		PatchObjects patch_objs;

    // The main loop to build PatchObject for all dependencies
    for (std::vector<sb::Symtab*>::iterator i = tabs.begin();
         i != tabs.end(); i++) {
			sb::Symtab* sym = *i;
			dt::Address load_addr = 0;
      al->getLoadAddress(sym, load_addr);

      // Parsing binary objects is very time consuming. We avoid parsing the
      // libraries that are either well known (e.g., libc, or dyninst).
      string libname_no_path = sp_filename(sym->name().c_str());
      if (is_dyninst_lib(libname_no_path) ||
          is_well_known_lib(libname_no_path)) {
#ifndef SP_RELEASE
        sp_debug("SKIPED - skip parsing %s", sp_filename(sym->name().c_str()));
#endif
        continue;
      }

      // Parse binary objects using ParseAPI::CodeObject::parse().
      // XXX: Should parse stripped binary also.
			pe::SymtabCodeSource* scs = new pe::SymtabCodeSource(sym);
      code_srcs_.push_back(scs);
			pe::CodeObject* co = new pe::CodeObject(scs);
      code_objs_.push_back(co);
      co->parse();
			/*
      vector<CodeRegion *>::const_iterator rit = scs->regions().begin();
      for( ; rit != scs->regions().end(); ++rit) {
        SymtabCodeRegion * scr = static_cast<SymtabCodeRegion*>(*rit);
        if(scr->symRegion()->isText()) {
          co->parseGaps(scr);
        }
      }
			*/

      // Create PatchObjects.
			dt::Address real_load_addr =
				load_addr ? load_addr : scs->loadAddress();
			ph::PatchObject* patch_obj = new sp::SpObject(co, load_addr,
                                                new SpCFGMaker,
                                                NULL,
                                                real_load_addr);
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

    // Initialize PatchAPI stuffs
    SpAddrSpace* as = SpAddrSpace::create(exe_obj_);

    ph::Instrumenter* inst = NULL;
    inst = sp::SpInstrumenter::create(as);

    sp::SpPointMaker* pm = new SpPointMaker;
    mgr_ = ph::PatchMgr::create(as, inst, pm);

    for (PatchObjects::iterator i = patch_objs.begin();
         i != patch_objs.end(); i++) {
      if (*i != exe_obj_) as->loadLibrary(*i);
    }
    well_known_libs_.insert(get_agent_name());

    return mgr_;
  }

  // Find the function that contains addr
	ph::PatchFunction*
  SpParser::findFunction(dt::Address addr) {
#if 0
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
#endif

#ifndef SP_RELEASE
    sp_debug("FIND FUNC BY ADDR - for call insn %lx", addr);
#endif
		ph::AddrSpace* as = mgr_->as();
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
      SpObject* obj = static_cast<SpObject*>(ci->second);

			pe::CodeObject* co = obj->co();
			pe::CodeSource* cs = co->cs();

			set<pe::CodeRegion *> match;
			set<pe::Block *> blocks;
			int cnt = cs->findRegions(addr, match);
			if(cnt != 1)
				continue;
			else if(cnt == 1) {
        co->findBlocks(*match.begin(), addr, blocks);

#ifndef SP_RELEASE
				sp_debug("%ld blocks found", (long)blocks.size());
#endif
				if (blocks.size() == 1) {
					std::vector<pe::Function*> funcs;
					(*blocks.begin())->getFuncs(funcs);
					return obj->getFunc(*funcs.begin());
				}
				return NULL;
			}
		}
		return NULL;
  }

  typedef struct {
    char libname[512];
    char err[512];
    char loaded;
    long pc;
    long sp;
    long bp;
  } IjMsg;

  // Get user-provided agent library's name.
  // We rely on looking for the magic variable -
  // " int self_propelled_instrumentation_agent_library = 19861985; "
  string
  SpParser::get_agent_name() {
    if (agent_name_.size() > 0) return agent_name_;

		ph::AddrSpace* as = mgr_->as();
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
			ph::PatchObject* obj = ci->second;
			pe::SymtabCodeSource* cs = (pe::SymtabCodeSource*)obj->co()->cs();
			sb::Symtab* sym = cs->getSymtabObject();
      if (!sym) {
        sp_perror("Failed to get Symtab object");
      }
      Symbols syms;
      string magic_var = "self_propelled_instrumentation_agent_library";
      if (sym->findSymbol(syms, magic_var) && syms.size() > 0) {
        agent_name_ = sym->name();
        break;
      }
    }
    return agent_name_;
  }

  // Get function address from function name.
  dt::Address
  SpParser::get_func_addr(string name) {
		ph::AddrSpace* as = mgr_->as();
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
			ph::PatchObject* obj = ci->second;
			pe::CodeObject* co = obj->co();
			pe::CodeObject::funclist& all = co->funcs();
      for (pe::CodeObject::funclist::iterator fit = all.begin();
           fit != all.end(); fit++) {
        if ((*fit)->name().compare(name) == 0) {
          dt::Address addr = (*fit)->addr() + obj->codeBase();
          return addr;
        }
      }
    }
    return 0;
  }

  // Find function by name.
	ph::PatchFunction*
  SpParser::findFunction(string name) {
#if 0
    if (real_func_map_.find(name) != real_func_map_.end()) {
      return real_func_map_[name];
    }

		ph::AddrSpace* as = mgr_->as();
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
				 ci != as->objMap().end(); ci++) {

			ph::PatchObject* obj = ci->second;
			pe::CodeObject* co = obj->co();
			pe::CodeObject::funclist& all = co->funcs();
			pe::SymtabCodeSource* cs = (pe::SymtabCodeSource*)obj->co()->cs();
			sb::Symtab* sym = cs->getSymtabObject();
      if (is_well_known_lib(sp_filename(sym->name().c_str()))) {
        continue;
      }

      for (pe::CodeObject::funclist::iterator fit = all.begin();
					 fit != all.end(); fit++) {
        if ((*fit)->name().compare(name) == 0) {
					pe::Region* region = sym->findEnclosingRegion((*fit)->addr());
          if (region && region->getRegionName().compare(".plt") == 0) {
            continue;
          }
					ph::PatchFunction* found = obj->getFunc(*fit);
          real_func_map_[name] = found;
          return found;
        }
      }
    }
    return NULL;
#endif
#ifndef SP_RELEASE
    sp_debug("LOOKING FOR FUNC - looking for %s", name.c_str());
#endif
    if (real_func_map_.find(name) != real_func_map_.end()) {
      return real_func_map_[name];
    }
    assert(mgr_);
		ph::AddrSpace* as = mgr_->as();
    FuncSet func_set;
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
			ph::PatchObject* obj = ci->second;
			pe::CodeObject* co = obj->co();
			pe::SymtabCodeSource* cs = (pe::SymtabCodeSource*)co->cs();
			sb::Symtab* sym = cs->getSymtabObject();
      if (!sym) {
        sp_perror("Failed to get Symtab object");
      }
#ifndef SP_RELEASE
      sp_debug("IN OBJECT - %s", sym->name().c_str());
#endif
      if (is_well_known_lib(sp_filename(sym->name().c_str()))) {
#ifndef SP_RELEASE
        sp_debug("SKIP - well known lib %s", sp_filename(sym->name().c_str()));
#endif
        continue;
      }

			pe::CodeObject::funclist& all = co->funcs();
      for (pe::CodeObject::funclist::iterator fit = all.begin();
           fit != all.end(); fit++) {

        if ((*fit)->name().compare(name) == 0) {
					sb::Region* region = sym->findEnclosingRegion((*fit)->addr());
          if (region && region->getRegionName().compare(".plt") == 0) {

#ifndef SP_RELEASE
            sp_debug("A PLT, SKIP - %s at %lx", name.c_str(), (*fit)->addr());
#endif
            continue;
          }
					ph::PatchFunction* found = obj->getFunc(*fit);
          if (real_func_map_.find(name) == real_func_map_.end())
            real_func_map_[name] = found;
          func_set.insert(found);
        }
      } // For each function
    } // For each object

    if (func_set.size() == 1) {
      return *func_set.begin();
    }
#ifndef SP_RELEASE
    sp_debug("NO FOUND - %s", name.c_str());
#endif
    return NULL;
  }

  // Dump instructions in text.
  string
  SpParser::dump_insn(void* addr, size_t size) {

    dt::Address base = (dt::Address)addr;
		pe::SymtabCodeSource* cs = (pe::SymtabCodeSource*)exe_obj_->co()->cs();
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
    SpVisitor(sp::SpPoint* pt)
      : Visitor(), call_addr_(0), pt_(pt), use_pc_(false) { }
    virtual void visit(in::RegisterAST* r) {
      if (is_pc(r->getID())) {
        use_pc_ = true;
        call_addr_ = pt_->block()->end();
        // call_addr_ = pt_->block()->last();
      } else {
        // Non-pc case, x86-32 always goes this way
        dt::Address rval = pt_->snip()->get_saved_reg(r->getID());
        call_addr_ = rval;
      }

#ifndef SP_RELEASE
			sp_debug("SP_VISITOR - reg value %lx", call_addr_);
#endif
      stack_.push(call_addr_);
    }
    virtual void visit(in::BinaryFunction* b) {
      dt::Address i1 = stack_.top();
      stack_.pop();
      dt::Address i2 = stack_.top();
      stack_.pop();

      if (b->isAdd()) {
        call_addr_ = i1 + i2;
#ifndef SP_RELEASE
				sp_debug("SP_VISITOR - %lx + %lx = %lx", i1, i2, call_addr_);
#endif
      } else if (b->isMultiply()) {
        call_addr_ = i1 * i2;
#ifndef SP_RELEASE
				sp_debug("SP_VISITOR - %lx * %lx = %lx", i1, i2, call_addr_);
#endif
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

#ifndef SP_RELEASE
			sp_debug("SP_VISITOR - imm %lx ", call_addr_);
#endif
      stack_.push(call_addr_);
    }
    virtual void visit(in::Dereference* d) {
      dt::Address* addr = (dt::Address*)stack_.top();
      stack_.pop();
      call_addr_ = *addr;
#ifndef SP_RELEASE
			sp_debug("SP_VISITOR - dereference %lx => %lx ",
							 (dt::Address)addr, call_addr_);
#endif
      stack_.push(call_addr_);
    }

    dt::Address call_addr() const {
      return call_addr_;
    }
    bool use_pc() const {
      return use_pc_;
    }
  private:
    std::stack<dt::Address> stack_;
    dt::Address call_addr_;
    sp::SpPoint* pt_;
    bool use_pc_;
  };

  // TODO (wenbin): is it okay to cache indirect callee?
	SpFunction*
  SpParser::callee(SpPoint* pt,
									 bool parse_indirect) {
		assert(pt);
    // 0. Check the cache
    // TODO: Should always re-parse indirect call
    if (pt->callee()) return FUNC_CAST(pt->callee());

    // 1. Looking for direct call
		ph::PatchFunction* f = pt->getCallee();
    if (f) {

			ph::PatchFunction* tmp_f = g_parser->findFunction(f->name());
      if (tmp_f && tmp_f != f)  f = tmp_f; 

			SpFunction* sfunc = FUNC_CAST(f);
			assert(sfunc);
      pt->set_callee(sfunc);
      return sfunc;
    } 
    else if (pt->callee()) {
      return FUNC_CAST(pt->callee());
    }

    // 2. Looking for indirect call
    if (parse_indirect) {
			SpBlock* b = pt->get_block();
			assert(b);

      sp_debug("PARSING INDIRECT - for call insn %lx",
							 b->last());

			in::Instruction::Ptr insn = b->orig_call_insn();
			assert(insn);

      sp_debug("DUMP INDCALL INSN (%ld bytes)- {", (long)insn->size());
      sp_debug("%s",
               dump_insn((void*)insn->ptr(),
                         insn->size()).c_str());
      sp_debug("DUMP INSN - }");

			in::Expression::Ptr trg = insn->getControlFlowTarget();
      dt::Address call_addr = 0;
      if (trg) {
        SpVisitor visitor(pt);
        trg->apply(&visitor);
        call_addr = visitor.call_addr();
        f = findFunction(call_addr);
        if (f) {
					SpFunction* sfunc = FUNC_CAST(f);
					assert(sfunc);
					sp_debug("PARSED INDIRECT - %lx is %s in %s", b->last(),
									 sfunc->name().c_str(),
									 sfunc->get_object()->name().c_str());

          pt->set_callee(sfunc);
          return sfunc;
        }
      }

      sp_debug("CANNOT FIND INDRECT CALL - for call insn %lx",
               b->last());

#if 0
      sp_print("DUMP INSN (%d bytes)- {", pt->snip()->size());
      sp_print("%s",
               dump_insn((void*)pt->snip()->buf(),
                         pt->snip()->size()).c_str());
      sp_print("DUMP INSN - }");
#endif
      return NULL;
    }

    return NULL;
  }

  void SpParser::get_frame(long* pc, long* sp, long* bp) {
    IjMsg* shm = (IjMsg*)SpInjector::get_shm(1986, sizeof(IjMsg));
    *pc = shm->pc;
    *sp = shm->sp;
    *bp = shm->bp;
  }
}
