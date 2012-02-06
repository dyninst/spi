#ifndef SP_PARSER_H_
#define SP_PARSER_H_

#include "SpAgentCommon.h"

namespace sp {

	class SpPoint;
	class SpObject;
	class SpFunction;

	typedef struct {
		dt::Address start;
		dt::Address end;
		dt::Address offset;
		string dev;
		unsigned long inode;
		int perms;
		string path;
	} MemMapping;
  typedef std::map<dt::Address, MemMapping> MemMappings;

	// -----------------------------------------------------------------------
	// Parser is to parse the CFG structures of the mutatee process.
	// This is a default implementation, which parses binary during runtime.
	// -----------------------------------------------------------------------
	class SpParser : public ph::CFGMaker {
  public:
    typedef dyn_detail::boost::shared_ptr<SpParser> ptr;

    virtual ~SpParser();
    static ptr create();

		// The main parsing procedure
    virtual ph::PatchMgrPtr parse();

		// Get the PatchObject that represents the executable
    ph::PatchObject* exe_obj();

		// Get the agent library's name
    string get_agent_name();

		// Get register values to form a stack frame
    void get_frame(long* pc, long* sp, long* bp);

		// Find function by absolute address
    ph::PatchFunction* findFunction(dt::Address addr);

		// Find function by its name
    ph::PatchFunction* findFunction(string name);

		// Get callee from a call point
    SpFunction* callee(SpPoint* pt,
											 bool     parse_indirect = false);

		// Get function absolute address from function name
    dt::Address get_func_addr(string name);

		// Dump instructions from a buffer
    string dump_insn(void* addr, size_t size);

		// Check if this agent library is injected (true) or is preloaded (false)
    bool injected() const { return injected_; }

		// Check if the library is a dyninst library (lib name is w/o path)
    bool is_dyninst_lib(string lib);

		// Check if the library is a well known library (lib name is w/o path)
    bool is_well_known_lib(string lib);

		ph::PatchMgrPtr mgr() const { return mgr_; }

		MemMappings& mem_maps() { return mem_maps_; }

		bool get_shared_libs();
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

		// Methods
    SpParser();
    void init_dyninst_libs();
		void init_well_known_libs();

    void update_mem_maps();
    void dump_mem_maps();
	};

}

#endif /* SP_PARSER_H_ */
