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

	// -------------------------------------------------------------------
	// Parser is to parse the CFG structures of the mutatee process.
	// This is a default implementation, which parses binary during runtime.
	// -------------------------------------------------------------------
	class SpParser : public ph::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<SpParser> ptr;

    virtual ~SpParser();
    static ptr create();

		// The main parsing procedure
    virtual ph::PatchMgrPtr parse();

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

		// Check if the library is a dyninst library (lib name is w/o path)
    bool is_dyninst_lib(string lib);

		// Check if the library is a well known library (lib name is w/o
		// path)
    bool is_well_known_lib(string lib);

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

    StringSet dyninst_libs_;
    StringSet well_known_libs_;

    RealFuncMap real_func_map_;

    MemMappings mem_maps_;
		FreeIntervalList free_intervals_;

		// Methods
    SpParser();
    void init_dyninst_libs();
		void init_well_known_libs();

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
