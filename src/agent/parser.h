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
  typedef std::map<std::string, SpFunction*> RealFuncMap;
  typedef std::set<sp::SpFunction*> FuncSet;
  
	class  SpParser {
  public:
    typedef SHARED_PTR(SpParser) ptr;
    AGENT_EXPORT static ptr Create();

    virtual ~SpParser();

		// The main parsing procedure
    AGENT_EXPORT virtual ph::PatchMgrPtr
        Parse();

    // Control what to instrument, what not to
    AGENT_EXPORT void
        SetLibrariesToInstrument(const StringSet& libs);
    AGENT_EXPORT bool
        CanInstrumentLib(const string& lib_full_path);
    AGENT_EXPORT void
        SetFuncsNotToInstrument(const StringSet& funcs);
    AGENT_EXPORT bool
        CanInstrumentFunc(const string& func_full_name);

    // Getters
    AGENT_EXPORT SpObject* exe_obj() const {
      return exe_obj_;
    }
    AGENT_EXPORT string agent_name() const {
      return agent_name_;
    }
		AGENT_EXPORT ph::PatchMgrPtr mgr() const {
      return mgr_;
    }

		// Check if this agent library is injected (true) or is
    // preloaded (false)
    AGENT_EXPORT bool injected() const {
      return injected_;
    }

		// Get register values to form a stack frame
    AGENT_EXPORT void GetFrame(long* pc,
                               long* sp,
                               long* bp);

    AGENT_EXPORT SpFunction*
        FindFunction(dt::Address absolute_addr);
    AGENT_EXPORT SpFunction*
        FindFunction(string func_name_without_path,
                     dt::Address addr = 0);
    AGENT_EXPORT SpFunction*
        callee(SpPoint* point,
               bool parse_indirect = false);
    AGENT_EXPORT dt::Address
        GetFuncAddrFromName(string func_name_without_path);

		// Dump instructions from a buffer
    AGENT_EXPORT string
        DumpInsns(void* addr,
                  size_t size);

  protected:
		// Is this agent library injected (true) or preloaded (false)?
    bool injected_;
    string agent_name_;

    CodeSources code_srcs_;
    CodeObjects code_objs_;

    ph::PatchMgrPtr mgr_;
    SpObject* exe_obj_;
    StringSet binaries_to_inst_;
    StringSet funcs_not_to_inst_;
    RealFuncMap real_func_map_;

    SpParser();

		// All about parsing
		sb::AddressLookup* GetRuntimeSymtabs(SymtabSet& symtabs);

		bool CreatePatchobjs(SymtabSet& symtabs,
												sb::AddressLookup* al,
												PatchObjects& patch_objs);

		SpObject* CreateObject(sb::Symtab* symtab,
													dt::Address load_addr);
		SpObject* CreateObjectFromRuntime(sb::Symtab* symtab,
																				 dt::Address load_addr);
		SpObject* CreateObjectFromFile(sb::Symtab* symtab,
																			dt::Address load_addr);

		SpObject* GetExeFromProcfs(PatchObjects& patch_objs);

		ph::PatchMgrPtr CreateMgr(PatchObjects& patch_objs);

    bool GetFuncsByName(sp::SpObject* obj,
                        std::string name,
                        dt::Address addr,
                        sp::FuncSet* func_set);
	};

}

#endif /* SP_PARSER_H_ */
