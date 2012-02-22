#include <sys/resource.h>

#include "agent/agent.h"
#include "agent/context.h"
#include "agent/addr_space.h"
#include "common/utils.h"
#include "injector/injector.h"

#include "patchAPI/h/PatchMgr.h"

namespace sp {

	// The only definition of global variables
	SpContext*    g_context = NULL;
	SpAddrSpace*  g_as = NULL;
	SpParser::ptr g_parser;
	int g_propel_lock;

	// Constructor for SpAgent
	SpAgent::ptr
	SpAgent::create() {

		// Enable core dump.
		if (getenv("SP_COREDUMP")) {
			struct rlimit core_limit;
			core_limit.rlim_cur = RLIM_INFINITY;
			core_limit.rlim_max = RLIM_INFINITY;
			if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
				sp_perror("ERROR: failed to setup core dump ability\n");
			}
		}
		return ptr(new SpAgent());
	}

	SpAgent::SpAgent() {

		parser_ = SpParser::ptr();
		init_event_ = SpEvent::ptr();
		fini_event_ = SpEvent::ptr();

		allow_ipc_ = false;
		trap_only_ = false;
		parse_only_ = false;
		directcall_only_ = false;
	}

	SpAgent::~SpAgent() {
	}

	// Configuration
	void
	SpAgent::set_parser(SpParser::ptr parser) {
		parser_ = parser;
	}

	void
	SpAgent::set_init_event(SpEvent::ptr e) {
		init_event_ = e;
	}

	void
	SpAgent::set_fini_event(SpEvent::ptr e) {
		init_event_ = e;
	}

	void
	SpAgent::set_init_entry(string p) {
		init_entry_ = p;
	}

	void
	SpAgent::set_init_exit(string p) {
		init_exit_ = p;
	}

	void
	SpAgent::set_init_propeller(SpPropeller::ptr p) {
		init_propeller_ = p;
	}

	void
	SpAgent::set_parse_only(bool b) {
		parse_only_ = b;
	}

	void
	SpAgent::set_directcall_only(bool b) {
		directcall_only_ = b;
	}

	void
	SpAgent::set_ipc(bool b) {
		allow_ipc_ = b;
	}

	void
	SpAgent::set_trap_only(bool b) {
		trap_only_ = b;
	}

	// Here We Go! Self-propelling magic happens!

	void
	SpAgent::go() {
		sp_debug("==== Start Self-propelled instrumentation @ Process %d ====",
						 getpid());

		// XXX: ignore bash/lsof/Injector for now ...
		StringSet illegal_exes;
		illegal_exes.insert("lsof");
		illegal_exes.insert("bash");
		illegal_exes.insert("Injector");
		illegal_exes.insert("sh");
		illegal_exes.insert("ssh");
		illegal_exes.insert("xauth");
		illegal_exes.insert("tcsh");
		illegal_exes.insert("scp");
		illegal_exes.insert("cp");
		illegal_exes.insert("netstat");

		if (sp::IsIllegalProgram(illegal_exes)) {
			sp_debug("ILLEGAL EXE - avoid instrumenting");
			return;
		}

		// Init lock
		InitLock(&g_propel_lock);

		if (getenv("SP_DIRECTCALL_ONLY")) directcall_only_ = true;
		if (getenv("SP_TRAP")) trap_only_ = true;

		// Sanity check. If not user-provided configuration, use default ones
		if (!init_event_) {
			sp_debug("INIT EVENT - Use default event");
			init_event_ = SyncEvent::create();
		}
		if (!fini_event_) {
			sp_debug("FINI EVENT - Use default event");
			fini_event_ = SpEvent::create();
		}
		if (init_entry_.size() == 0) {
			sp_debug("ENTRY_PAYLOAD - Use default payload entry calls");
			init_entry_ = "default_entry";
		}
		if (init_exit_.size() == 0) {
			sp_debug("EXIT_PAYLOAD - No payload exit calls");
			init_exit_ = "";
		}
		if (!parser_) {
			sp_debug("PARSER - Use default parser");
			parser_ = SpParser::create();
		}
		if (!init_propeller_) {
			sp_debug("PROPELLER - Use default propeller");
			init_propeller_ = SpPropeller::create();
		}

		if (directcall_only_) {
			sp_debug("DIRECT CALL ONLY - only instrument direct calls, ignoring indirect calls");
		} else {
			sp_debug("DIRECT/INDIRECT CALL - instrument both direct and indirect calls");
		}
		if (allow_ipc_) {
			sp_debug("MULTI PROCESS - support multiprocess instrumentation");
		} else {
			sp_debug("SINGLE PROCESS - only support single-process instrumentation");
		}
		if (trap_only_) {
			sp_debug("TRAP ONLY - Only use trap-based instrumentation");
		} else {
			sp_debug("JUMP + TRAP - Use jump and trap for instrumentation");
		}

		// Set up globally unique parser
		g_parser = parser_;
		assert(g_parser);

		// Prepare context
		g_context = SpContext::create(init_propeller_,
																	init_entry_,
																	init_exit_,
																	parser_);
		assert(g_context && g_parser->mgr());

		g_as = AS_CAST(g_parser->mgr()->as());
		assert(g_as);

		g_context->set_directcall_only(directcall_only_);
		g_context->set_allow_ipc(allow_ipc_);

		if (parse_only_) {
			sp_debug("PARSE ONLY - exit after parsing, without instrumentation");
			return;
		}

		// Register Events
		init_event_->register_event();
		fini_event_->register_event();
	}

}
