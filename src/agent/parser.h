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

// Parser is to parse the CFG structures of the mutatee process.

#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "common/common.h"

#include "common/utils.h"

#include "patchAPI/h/CFGMaker.h"

namespace sp {

	class SpPoint;
	class SpObject;
	class SpFunction;

  typedef std::set<sb::Symtab*> SymtabSet;
  typedef std::vector<pe::CodeObject*> CodeObjects;
  typedef std::vector<pe::CodeSource*> CodeSources;
	typedef std::vector<sb::Symbol*> Symbols;
  typedef std::vector<ph::PatchObject*> PatchObjects;
  typedef std::map<std::string, ph::PatchFunction*> RealFuncMap;
  
	typedef struct {
		dt::Address previous_end;  // previous object's end addr
		dt::Address start;
		dt::Address end;
		dt::Address offset;
    std::string dev;
		unsigned long inode;
		int perms;
		string path;
	} MemMapping;
  typedef std::map<dt::Address, MemMapping> MemMappings;

	typedef struct {
		dt::Address start;
		dt::Address end;
		dt::Address size() { return (end - start); }
		bool used;
	} FreeInterval;
	typedef std::list<FreeInterval> FreeIntervalList;

	class AGENT_EXPORT SpParser : public ph::CFGMaker {
  public:
    typedef SHARED_PTR(SpParser) ptr;

    virtual ~SpParser();
    static ptr create();

		// The main parsing procedure
    virtual ph::PatchMgrPtr parse();

    // Libraries to instrument
    void SetLibrariesToInstrument(const StringSet& libs);
    
		// Get the PatchObject that represents the executable
    ph::PatchObject* exe_obj() const { return exe_obj_; }

		// Get the agent library's name
    string get_agent_name();

		// Get register values to form a stack frame
    void get_frame(long* pc, long* sp, long* bp);

		// Find function by absolute address
    ph::PatchFunction* findFunction(dt::Address addr);

		// Find function by its name
    ph::PatchFunction* findFunction(string name, bool allow_plt=false);

		// Get callee from a call point
    SpFunction* callee(SpPoint* pt,
											 bool     parse_indirect = false);

		// Get function absolute address from function name
    dt::Address get_func_addr(string name);

		// Dump instructions from a buffer
    string dump_insn(void* addr, size_t size);

		// Check if this agent library is injected (true) or is
    // preloaded (false)
    bool injected() const { return injected_; }

    // Checks whether this library is okay to be instrumented
    bool CanInstrument(string lib_full_path);
    
		ph::PatchMgrPtr mgr() const { return mgr_; }

		MemMappings& mem_maps() { return mem_maps_; }

  protected:
		// Is this agent library injected (true) or preloaded (false)?
    bool injected_;
    string agent_name_;

    CodeSources code_srcs_;
    CodeObjects code_objs_;

    ph::PatchMgrPtr mgr_;
    ph::PatchObject* exe_obj_;

    StringSet binaries_to_inst_;
        
    RealFuncMap real_func_map_;

    MemMappings mem_maps_;
		FreeIntervalList free_intervals_;

		// Methods
    SpParser();

    void update_mem_maps();
    void dump_mem_maps();
    void dump_free_intervals();
		bool get_closest_interval(dt::Address addr,
															FreeInterval** interval);

		// All about parsing
		sb::AddressLookup* get_runtime_symtabs(SymtabSet& symtabs);

		bool create_patchobjs(SymtabSet& symtabs,
													sb::AddressLookup* al,
													PatchObjects& patch_objs);

		SpObject* create_object(sb::Symtab* symtab,
														dt::Address load_addr);
		SpObject* create_object_from_runtime(sb::Symtab* symtab,
																				 dt::Address load_addr);
		SpObject* create_object_from_file(sb::Symtab* symtab,
																			dt::Address load_addr);

		SpObject* get_exe_from_procfs(PatchObjects& patch_objs);

		ph::PatchMgrPtr create_mgr(PatchObjects& patch_objs);

	};

}

#endif /* SP_PARSER_H_ */
