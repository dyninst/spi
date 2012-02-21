#include <sys/shm.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stack>

#include "Symtab.h"
#include "AddrLookup.h"

#include "SpMaker.h"
#include "SpUtils.h"
#include "SpPoint.h"
#include "SpParser.h"
#include "SpObject.h"
#include "SpContext.h"
#include "SpInjector.h"
#include "SpAddrSpace.h"
#include "SpInstrumenter.h"

#include "PatchMgr.h"

#include "Visitor.h"
#include "Immediate.h"
#include "BinaryFunction.h"

// Magic number to identify agent library
// ( make the name long enough ... )
int self_propelled_instrumentation_agent_library = 19861985;

namespace sp {

  extern SpContext* g_context;
	extern SpParser::ptr g_parser;

  SpParser::SpParser()
    : injected_(false), exe_obj_(NULL) {

    update_mem_maps();

    init_well_known_libs();
    init_dyninst_libs();

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

		// For chrome
    well_known_libs_.insert("libgtk-");
    well_known_libs_.insert("libgdk");
    well_known_libs_.insert("libcairo");       // Drawing
    well_known_libs_.insert("libpangocairo-");
    well_known_libs_.insert("libpango-");
    well_known_libs_.insert("libz.so");
    well_known_libs_.insert("libnss3.so");     // Network security service
    well_known_libs_.insert("libXext.so");
    well_known_libs_.insert("libnssutil3.so");
    well_known_libs_.insert("librt-");
    well_known_libs_.insert("libXss.so");
    well_known_libs_.insert("libXrender");
    well_known_libs_.insert("libglib-");
    well_known_libs_.insert("libX11");
    well_known_libs_.insert("libsmime3.so");
    well_known_libs_.insert("libnspr4.so");
    well_known_libs_.insert("libfontconfig.so");
    well_known_libs_.insert("libplc4");
    well_known_libs_.insert("libgthread-");
    well_known_libs_.insert("libfreetype.so");
    well_known_libs_.insert("libgobject-");
    well_known_libs_.insert("libgio-");
    well_known_libs_.insert("libatk-");
    well_known_libs_.insert("libXdamage.so");
    well_known_libs_.insert("libXfixes.so");
    well_known_libs_.insert("libcrypt-");
    well_known_libs_.insert("libXcomposite.so");
    well_known_libs_.insert("libXinerama.so");
    well_known_libs_.insert("libgpg-error.so");
    well_known_libs_.insert("libXcursor.so");
    well_known_libs_.insert("libXdmcp.so");
    well_known_libs_.insert("libXau.so");
    well_known_libs_.insert("libcrypto.so");
    well_known_libs_.insert("libexpat.so");
    well_known_libs_.insert("libdbus-glib-");
    well_known_libs_.insert("libdbus-");
    well_known_libs_.insert("libbz2.so");
    well_known_libs_.insert("libkeyutils-");
    well_known_libs_.insert("libcups.so");
    well_known_libs_.insert("libgcrypt.so");
    well_known_libs_.insert("libasound.so");
    well_known_libs_.insert("libssl.so");
    well_known_libs_.insert("libresolv-");
    well_known_libs_.insert("libpng14.so");
    well_known_libs_.insert("libgconf-");
    well_known_libs_.insert("libkrb5support.so");
    well_known_libs_.insert("libk5crypto.so");
    well_known_libs_.insert("libcom_err.so");
    well_known_libs_.insert("libkrb5.so");
    well_known_libs_.insert("libORBit-");
    well_known_libs_.insert("libgssapi_krb5.so");
    well_known_libs_.insert("libplds4.so");
    well_known_libs_.insert("libcom_err.so");
    well_known_libs_.insert("libpangoft2-1.0.so");
    well_known_libs_.insert("libgmodule-2.0.so");
    well_known_libs_.insert("libsepol.so");
    well_known_libs_.insert("libselinux.so");
    well_known_libs_.insert("libpixman-1.so");
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
  // This is the default implementation, which parses binary during 
	// runtime.
  // Agent-writer can provide their own parse() method.
	ph::PatchMgrPtr
  SpParser::parse() {
    // Avoid duplicate parsing. If a shared library is loaded during
    // runtime, we have to use PatchAPI's loadLibrary method to add
    // this shared library
    if (mgr_) return mgr_;

		// Step 1: Parse runtime symtabs
    SymtabSet unique_tabs;
		sb::AddressLookup* al = get_runtime_symtabs(unique_tabs);
		if (!al) {
			sp_perror("FAILED TO GET RUNTIME SYMTABS");
		}

		// Step 2: Create patchapi objects
		PatchObjects patch_objs;
		if (!create_patchobjs(unique_tabs, al, patch_objs)) {
			sp_perror("FAILED TO CREATE PATCHOBJS");
		}

		// Step 3: In the case of executable shared library, we cannot get
		// exe_obj_ via symtabAPI, so we resort to /proc
		if (!exe_obj_) {
			exe_obj_ = get_exe_from_procfs(patch_objs);
			if (!exe_obj_) {
				sp_perror("FAILED TO GET EXE OBJ FROM PROC FS");
			}
		} // !exe_obj_

    // Step 4: Initialize PatchAPI stuffs
		mgr_ = create_mgr(patch_objs);
		if (!mgr_) {
			sp_perror("FAILED TO CREATE PATCHMGR");
		}

		// Step 4: Add agent library's name here, because we need to parse
		// agent library for the payload function, but we don't want to
    // iterate agent library's functions to find a user function
    well_known_libs_.insert(agent_name_);

    return mgr_;
  }

  // Find the function that contains addr
	ph::PatchFunction*
  SpParser::findFunction(dt::Address addr) {
    sp_debug("FIND FUNC BY ADDR - for call insn %lx", addr);
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

				sp_debug("%ld blocks found", (long)blocks.size());
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
  string
  SpParser::get_agent_name() {
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
  // If allow_plt = true, then we may return a plt entry;
  // otherwise, we strictly skip plt entries.
	ph::PatchFunction*
  SpParser::findFunction(string name, bool allow_plt) {
    sp_debug("LOOKING FOR FUNC - looking for %s", name.c_str());
    if (real_func_map_.find(name) != real_func_map_.end()) {
      return real_func_map_[name];
    }
    assert(mgr_);
		ph::AddrSpace* as = mgr_->as();
    FuncSet func_set;
    for (ph::AddrSpace::ObjMap::iterator ci = as->objMap().begin();
         ci != as->objMap().end(); ci++) {
			SpObject* obj = OBJ_CAST(ci->second);
			pe::CodeObject* co = obj->co();
			sb::Symtab* sym = obj->symtab();
      if (!sym) {
        sp_perror("Failed to get Symtab object");
      }

      sp_debug("IN OBJECT - %s", sym->name().c_str());

      if (is_well_known_lib(sp_filename(sym->name().c_str()))) {
        sp_debug("SKIP - well known lib %s",
								 sp_filename(sym->name().c_str()));
        continue;
      }

			pe::CodeObject::funclist& all = co->funcs();
      for (pe::CodeObject::funclist::iterator fit = all.begin();
           fit != all.end(); fit++) {

        if ((*fit)->name().compare(name) == 0) {
					sb::Region* region = sym->findEnclosingRegion((*fit)->addr());
          if (!allow_plt && region &&
							(region->getRegionName().compare(".plt") == 0)) {

            sp_debug("A PLT, SKIP - %s at %lx", name.c_str(),
										 (*fit)->addr());
            continue;
          }
					ph::PatchFunction* found = obj->getFunc(*fit);
          if (real_func_map_.find(name) == real_func_map_.end())
            real_func_map_[name] = found;
					sp_debug("GOT %s in OBJECT - %s", name.c_str(),
									 sym->name().c_str());
          func_set.insert(found);
        }
      } // For each function
    } // For each object

    if (func_set.size() == 1) {
			sp_debug("FOUND - %s in object %s", name.c_str(),
        FUNC_CAST((*func_set.begin()))->get_object()->name().c_str());
      return *func_set.begin();
    }
    sp_debug("NO FOUND - %s", name.c_str());
    return NULL;
  }

  // Dump instructions in text.
  string
  SpParser::dump_insn(void* addr, size_t size) {

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
    SpVisitor(sp::SpPoint* pt)
      : Visitor(), call_addr_(0), pt_(pt), use_pc_(false) { }
    virtual void visit(in::RegisterAST* r) {
      if (IsPcRegister(r->getID())) {
        use_pc_ = true;
        call_addr_ = pt_->block()->end();
        // call_addr_ = pt_->block()->last();
      } else {
        // Non-pc case, x86-32 always goes this way
        dt::Address rval = pt_->snip()->get_saved_reg(r->getID());
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
      dt::Address* addr = (dt::Address*)stack_.top();
      stack_.pop();
      call_addr_ = *addr;
			sp_debug("SP_VISITOR - dereference %lx => %lx ",
							 (dt::Address)addr, call_addr_);
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

  // Parse /proc/pid/maps file to build memory mappings
  void
  SpParser::update_mem_maps() {

    char maps_file[256];
    sprintf(maps_file, "/proc/%d/maps", getpid());

    FILE* fp = fopen(maps_file, "r");
    if (!fp) {
      sp_perror("FAILED to open memory mapping file %s", maps_file);
    }

    char linebuf[2048];
		dt::Address previous_end = 0;

    while (fgets(linebuf, 2048, fp) != NULL) {
      char* start_addr_s = linebuf;
      char* end_addr_s = strchr(linebuf, '-');
      *end_addr_s = '\0';
      end_addr_s++;

      char* perms_s = strchr(end_addr_s, ' ');
      *perms_s = '\0';
      perms_s++;

      char* offset_s = strchr(perms_s, ' ');
      *offset_s = '\0';
      offset_s++;

      char* dev_s = strchr(offset_s, ' ');
      *dev_s = '\0';
      dev_s++;

      char* inode_s = strchr(dev_s, ' ');
      *inode_s = '\0';
      inode_s++;

      char* pch = strtok(inode_s, " \n\r");
      inode_s = pch;
      char* path_s = NULL;
      if (pch != NULL) {
        pch = strtok(NULL, " \n\r");
        if (pch) {
          path_s = pch;
        }
      }

      char* pDummy;
      dt::Address start = strtoll(start_addr_s, &pDummy, 16);


      if (mem_maps_.find(start) == mem_maps_.end()) {
        MemMapping& mapping = mem_maps_[start];
				mapping.previous_end = previous_end;
        mapping.start = start;
        mapping.end = strtol(end_addr_s, &pDummy, 16);
        mapping.offset = strtol(offset_s, &pDummy, 16);
        mapping.dev = dev_s;
        mapping.inode = strtol(inode_s, &pDummy, 16);

				if (previous_end != mapping.start) {
					FreeInterval interval;
					interval.start = previous_end;
					interval.end = mapping.start;
					interval.used = false;
					free_intervals_.push_back(interval);
				}

				previous_end = mapping.end;
        if (path_s) mapping.path = path_s;
        int perms = 0;
        int count = 0;
        if (perms_s[0] == 'r') {
          perms |= PROT_READ;
          ++count;
        }
        if (perms_s[1] == 'w') {
          perms |= PROT_WRITE;
          ++count;
        }
        if (perms_s[2] == 'x') {
          perms |= PROT_EXEC;
          ++count;
        }
        if (count == 3) perms = PROT_NONE;
        mapping.perms = perms;
      }
    }
    fclose(fp);
  }

  void
  SpParser::dump_mem_maps() {

    sp_debug("MMAPS - %lu memory mappings",
						 (unsigned long)mem_maps_.size());

    for (MemMappings::iterator mi = mem_maps_.begin();
				 mi != mem_maps_.end(); mi++) {

      MemMapping& mapping = mi->second;
      sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s,"
							 " Inode %lu, Path %s, previous_end %lx",
               mapping.start, mapping.end, mapping.offset,
							 mapping.perms, mapping.dev.c_str(), mapping.inode,
							 mapping.path.c_str(),
							 mapping.previous_end);
    }
  }

	// Get symtabs during runtime
	// Side effect:
	// 1. determine whether this agent is injected or preloaded
  // 2. figure out the agent library's name
	sb::AddressLookup*
	SpParser::get_runtime_symtabs(SymtabSet& symtabs) {
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
		sp_debug("SYMTABS - %ld symtabs found", tabs.size());

    // Determine whether the agent is injected or preloaded.
    //   - true: injected by other process (libijagent.so is loaded)
    //   - false: preloaded

		// We rely on looking for the magic variable -
		// " int self_propelled_instrumentation_agent_library = 19861985; "
		string magic_var = "self_propelled_instrumentation_agent_library";

    for (std::vector<sb::Symtab*>::iterator i = tabs.begin();
         i != tabs.end(); i++) {
			sb::Symtab* sym = *i;

			// Figure the name for agent library
			Symbols syms;
			if (sym->findSymbol(syms, magic_var) && syms.size() > 0) {
				agent_name_ = sym->name();
			}

			// Deduplicate symtabs
			symtabs.insert(sym);
      if (sym->name().find("libijagent.so") != string::npos) {
        injected_ = true;
      }
    }
		return al;
	}

	bool
	SpParser::create_patchobjs(SymtabSet& unique_tabs,
														 sb::AddressLookup* al,
														 PatchObjects& patch_objs) {
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
      if (is_dyninst_lib(libname_no_path) ||
          is_well_known_lib(libname_no_path)) {
        sp_debug("SKIPED - skip parsing %s",
								 sp_filename(sym->name().c_str()));
        continue;
      }

			ph::PatchObject* patch_obj = create_object(sym, load_addr);
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

	SpObject*
	SpParser::get_exe_from_procfs(PatchObjects& patch_objs) {
		for (PatchObjects::iterator oi = patch_objs.begin();
				 oi != patch_objs.end(); oi++) {
			SpObject* obj = OBJ_CAST(*oi);
			assert(obj);
			char* s1 = sp_filename(sp::get_exe_name().c_str());
			char* s2 = sp_filename(obj->name().c_str());
			// sp_debug("PARSE EXE - s1=%s, s2 =%s", s1, s2);
			if (strcmp(s1, s2) == 0) {
				sp_debug("GOT EXE - %s is an executable shared library", s2);
				return obj;
			} // strcmp
		} // for each obj
		return NULL;
	}

	// Create a PatchMgr object.
	ph::PatchMgrPtr
	SpParser::create_mgr(PatchObjects& patch_objs) {
		SpAddrSpace* as = SpAddrSpace::create(exe_obj_);
		assert(as);

    SpInstrumenter* inst = sp::SpInstrumenter::create(as);
		assert(inst);

		ph::PatchMgrPtr mgr = ph::PatchMgr::create(as,
																							 inst,
																							 new SpPointMaker);
		assert(mgr);

		dump_mem_maps();
		dump_free_intervals();
    for (PatchObjects::iterator i = patch_objs.begin();
         i != patch_objs.end(); i++) {
      if (*i != exe_obj_) as->loadLibrary(*i);

			SpObject* obj = static_cast<SpObject*>(*i);
			assert(obj);

			// Bind preallocated close free buffers to each object

			sp_debug("HANDLING OBJECT - %s @ load addr: %lx, code base: %lx",
							 obj->name().c_str(), obj->load_addr(), obj->codeBase());
      MemMapping& mapping = mem_maps_[obj->load_addr()];
      sp_debug("MMAP - Range[%lx ~ %lx], Offset %lx, Perm %x, Dev %s,"
							 " Inode %lu, Path %s, previous_end %lx",
               mapping.start, mapping.end, mapping.offset,
							 mapping.perms, mapping.dev.c_str(), mapping.inode,
							 mapping.path.c_str(),
							 mapping.previous_end);

			FreeInterval* interval = NULL;
			if (!get_closest_interval(mapping.start, &interval)) {
				sp_debug("FAILED TO GET FREE INTERVAL - for %lx %s",
								 mapping.start, obj->name().c_str());
				continue;
			}
			assert(interval);
			size_t size = interval->size();
			size_t ps = getpagesize();

			sp_debug("GET FREE INTERVAL - [%lx, %lx], w/ original size %ld, "
							 "rounded size %ld", interval->start, interval->end,
							 interval->size(), size);

			size = (size <= 2147483646 ? size : 2147483646);
      size = ((size + ps -1) & ~(ps - 1));

			dt::Address base = interval->end - size;
			obj->init_memory_alloc(base, size);
    }

		return mgr;
	}

	SpObject*
	SpParser::create_object(sb::Symtab* sym,
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
			obj = create_object_from_file(sym, load_addr);
		} else {
			sp_debug("PARSE FROM RUNTIME - for %s", sym->name().c_str());
			obj = create_object_from_runtime(sym, load_addr);
		}
		return obj;
	}

	SpObject*
	SpParser::create_object_from_runtime(sb::Symtab* sym,
																			 dt::Address load_addr) {
	
		// Parse binary objects using ParseAPI::CodeObject::parse().
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

		dt::Address real_load_addr =
			load_addr ? load_addr : scs->loadAddress();

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
	SpParser::create_object_from_file(sb::Symtab* sym,
																		dt::Address load_addr) {
		SpObject* obj = NULL;
		return obj;
	}

	void
	SpParser::dump_free_intervals() {
		for (FreeIntervalList::iterator i = free_intervals_.begin();
				 i != free_intervals_.end(); i++) {
			sp_debug("FREE INTERVAL - [%lx ~ %lx], used %d",
							 (*i).start, (*i).end, (*i).used);
		}
	}

	bool
	SpParser::get_closest_interval(dt::Address addr,
																 FreeInterval** interval) {
		FreeInterval* previous = NULL;
		for (FreeIntervalList::iterator i = free_intervals_.begin();
				 i != free_intervals_.end(); i++) {
			if ((*i).end <= addr) {
				previous = &(*i);
				continue;
			} else {
				if (!previous) return false;
				if (previous->used) return false;
				*interval = previous;
				(*interval)->used = true;
				return true;
			}
		}
		return false;
	} // get_closest_interval
}
