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

#include <sys/resource.h>
#include <errno.h>

#include "agent/agent.h"
#include "agent/context.h"
#include "agent/patchapi/addr_space.h"
#include "agent/patchapi/object.h"
#include "common/utils.h"
#include "common/common.h"
#include "injector/injector.h"
#include <signal.h>
#include <fcntl.h>
#include <sys/ipc.h>  // clean shared memory if created
#include <sys/shm.h>
#include "PatchMgr.h"
FILE* g_debug_fp;
FILE* g_error_fp;
FILE* g_output_fp;

namespace sp {

  // The only definition of global variables
  SpAddrSpace*  g_as = NULL;
  SpContext*    g_context = NULL;
  SpParser::ptr g_parser;

  extern SpLock g_propel_lock;

  // Constructor for SpAgent
  SpAgent::ptr
  SpAgent::Create() {

    // Enable core dump.
    if (getenv("SP_COREDUMP")) {
      struct rlimit core_limit;
      core_limit.rlim_cur = RLIM_INFINITY;
      core_limit.rlim_max = RLIM_INFINITY;
      if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
        sp_perror("ERROR: failed to setup core dump ability\n");
      }
    }

    // initialize runtime root path for SPI
    std::string runtime_dir;
    if (getenv("SP_DIR") && getenv("PLATFORM")) {
      runtime_dir = getenv("SP_DIR");
      runtime_dir += "/";
      runtime_dir += getenv("PLATFORM");
    } else {
      runtime_dir = "/";
    }
	
    // open files for debug, error, output
    std::ostringstream error_file, output_file;
    error_file << runtime_dir << "/tmp/spi/spi-error-" << getpid();
    output_file << runtime_dir << "/tmp/spi/spi-output-" << getpid();
    g_error_fp = fopen(error_file.str().c_str(), "a+");
    if (g_error_fp == NULL) {
      std::cerr << "Failed to open file for output erro info: " << strerror(errno) << std::endl;
      std::cerr << "Using stderr instead...\n";
      g_error_fp = stderr;
    }

    g_output_fp = fopen(output_file.str().c_str(), "a+");
    if (g_output_fp == NULL) {
      std::cerr << "Failed to open file for output stdout info: " << strerror(errno) << std::endl;
      std::cerr << "Using stderr instead...\n";
      g_output_fp = stderr;
    }

    // Enalbe outputing debug info to /tmp/spi-$PID
    if (getenv("SP_FDEBUG")) {
      char fn[255];
      snprintf(fn, 255, "%s/%s/tmp/spi/spi-%d", getenv("SP_DIR"), getenv("PLATFORM"), getpid());
      g_debug_fp = fopen(fn, "a+");
      if (g_debug_fp == NULL) {
        std::cerr << "Failed to open file for output debug info: " << strerror(errno) << std::endl;
        std::cerr << "Using stderr instead...\n";
        g_debug_fp = stderr;
      }
    }
    return ptr(new SpAgent());
  }
 
  SpAgent::SpAgent() {

    parser_ = SpParser::ptr();
    init_event_ = SpEvent::ptr();
    fini_event_ = SpEvent::ptr();
    context_ = NULL;

    allow_ipc_ = false;
    allow_multithread_ = false;
    trap_only_ = false;
    parse_only_ = false;
    handle_dlopen_ = false;
    directcall_only_ = false;
  }

  SpAgent::~SpAgent() {
  }

  // Configuration
  void
  SpAgent::SetParser(SpParser::ptr parser) {
    parser_ = parser;
  }

  void
  SpAgent::SetInitEvent(SpEvent::ptr e) {
    init_event_ = e;
  }

  void
  SpAgent::SetFiniEvent(SpEvent::ptr e) {
    fini_event_ = e;
  }

  void
  SpAgent::SetInitEntry(string p) {
    init_entry_ = p;
  }

  void
  SpAgent::SetInitExit(string p) {
    init_exit_ = p;
  }

  void
  SpAgent::SetInitPropeller(SpPropeller::ptr p) {
    init_propeller_ = p;
  }

  void
  SpAgent::EnableParseOnly(const bool b) {
    parse_only_ = b;
  }

  void
  SpAgent::EnableDirectcallOnly(const bool b) {
    directcall_only_ = b;
  }

  void
  SpAgent::EnableIpc(const bool b) {
    allow_ipc_ = b;
  }

  void
  SpAgent::EnableHandleDlopen(const bool b) {
    handle_dlopen_ = b;
  }

  void
  SpAgent::EnableMultithread(const bool b) {
    allow_multithread_ = b;
  }

  void
  SpAgent::EnableTrapOnly(const bool b) {
    trap_only_ = b;
  }

  void
  SpAgent::SetLibrariesNotToInstrument(const StringSet& libs) {
    for (StringSet::iterator i = libs.begin(); i != libs.end(); i++)
      libs_not_to_inst_.insert(*i);
  }

  void
  SpAgent::SetFuncsNotToInstrument(const StringSet& funcs) {
    for (StringSet::iterator i = funcs.begin(); i != funcs.end(); i++)
      funcs_not_to_inst_.insert(*i);
  }

  // Here We Go! Self-propelling magic happens!

  void
  SpAgent::Go() {
    sp_debug("==== Start Self-propelled instrumentation @ Process %d ====",
             getpid());

    // XXX: ignore bash/lsof/Injector for now ...
    if (sp::IsIllegalProgram()) {
      sp_debug("ILLEGAL EXE - avoid instrumenting %s", GetExeName().c_str());
      return;
    }

    // Init lock
    InitLock(&g_propel_lock);

   		  
    /*  Uncomment this if you need to do flow analysis
    // Register a signal handler
    signal(SIGURG, sig_urg_handler);
    
    //fcntl all the sockets which this process use
    SocketSet socket_set; 
    sp::GetSocketDescFromPid(getpid(),socket_set); 
    if(socket_set.size() > 0) {
	for(SocketSet::iterator s=socket_set.begin(); s!=socket_set.end(); s++) {
		fcntl(*s, F_SETOWN,getpid());
	}
	
    }
    */
    // For quick debugging
    if (getenv("SP_DIRECTCALL_ONLY")) {
      directcall_only_ = true;
    }

    if (getenv("SP_TRAP")) {
      trap_only_ = true;
    }

    // Sanity check. If not user-provided configuration, use default ones
    if (!init_event_) {
      sp_debug("INIT EVENT - Use default event");
      init_event_ = SyncEvent::Create();
    }
    if (!fini_event_) {
      sp_debug("FINI EVENT - Use default event");
      fini_event_ = SpEvent::Create();
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
      parser_ = SpParser::Create();
    }
    if (!init_propeller_) {
      sp_debug("PROPELLER - Use default propeller");
      init_propeller_ = SpPropeller::Create();
    }

    if (directcall_only_) {
      sp_debug("DIRECT CALL ONLY - only instrument direct calls,"
               " ignoring indirect calls");
    } else {
      sp_debug("DIRECT/INDIRECT CALL - instrument both direct and"
               " indirect calls");
    }
    if (allow_ipc_) {
      sp_debug("MULTI PROCESS - support multiprocess instrumentation");
    } else {
      sp_debug("SINGLE PROCESS - only support single-process "
               "instrumentation");
    }
    if (trap_only_) {
      sp_debug("TRAP ONLY - Only use trap-based instrumentation");
    } else {
      sp_debug("JUMP + TRAP - Use jump and trap for instrumentation");
    }


    // Set up globally unique parser
    // The parser will be freed automatically, because it is a shared ptr
    g_parser = parser_;
    assert(g_parser);
    g_parser->SetLibrariesNotToInstrument(libs_not_to_inst_);
    g_parser->SetFuncsNotToInstrument(funcs_not_to_inst_);

    //Parse and store the necessary` datastructures
    parser_->Parse();
    assert(g_parser->mgr());

    // We use default event, which does nothing on initial instrumentation
    if (parse_only_) {
      sp_debug("PARSE ONLY - exit after parsing, without instrumentation");
      sp::SpEvent::ptr init_event = sp::SpEvent::Create();
      init_event_  = init_event;
      // return;
    }

    // Prepare context
    // XXX: this never gets freed ... should free it when unloading
    //      this shared agent library?
    g_context = SpContext::Create();
    context_ = g_context;
    assert(g_context);

    // Set up globally unique address space object
    // This will be freed in SpContext::~SpContext
    g_as = AS_CAST(g_parser->mgr()->as());
    assert(g_as);


    // Copy agent's variables to context
    sp_debug("Copy agent's variables to context");
    g_context->SetInitPropeller(init_propeller_);
    g_context->SetParser(parser_);
    g_context->SetInitEntryName(init_entry_);
    void* payload_entry =
        (void*)g_parser->GetFuncAddrFromName(init_entry_);
    assert(payload_entry);
    g_context->SetInitEntry(payload_entry);

    if (init_exit_.size() == 0 && allow_ipc_) {
      init_exit_ = "default_exit";
    }
    
    if (init_exit_.size() > 0) {
      g_context->SetInitExitName(init_exit_);
      void* payload_exit =
          (void*)g_parser->GetFuncAddrFromName(init_exit_);
      assert(payload_exit);
      g_context->SetInitExit(payload_exit);
    }
    
    g_context->EnableDirectcallOnly(directcall_only_);
    g_context->EnableIpc(allow_ipc_);
    g_context->EnableHandleDlopen(handle_dlopen_);
    g_context->EnableMultithread(allow_multithread_);
    if (allow_ipc_) {
      // SpIpcMgr will be deleted in the destructor of SpContext
      g_context->SetIpcMgr(new SpIpcMgr());
    }

    // Always use wrapper functions for payload entry and exit
    sp_debug("ALLOW IPC OR MULTITHREADED OR HANDLE_DLOPEN");
    void* wrapper_entry =
        (void*)g_parser->GetFuncAddrFromName("wrapper_entry");
    assert(wrapper_entry);
    g_context->SetWrapperEntry(wrapper_entry);
    void* wrapper_exit =
        (void*)g_parser->GetFuncAddrFromName("wrapper_exit");
    assert(wrapper_exit);
    g_context->SetWrapperExit(wrapper_exit);

    // Register Events for initial instrumentation
    init_event_->RegisterEvent();
    sp_debug("init registered");
    fini_event_->RegisterEvent();
    sp_debug("fini registered");

    std::string exit_function_payload_str("toggle_off_instrumentation_entry");
    void* exit_function_payload = (void*)g_parser->GetFuncAddrFromName(exit_function_payload_str);

    if (exit_function_payload == NULL) {
      sp_perror("Failed to find exit function payload to toggle off instrumentation, return immediately");
      return;
    }

    // Instrument exit function inside libc to stop SPI when exit is called
    // Instrument _dl_runtime_resolve inside ld.so
    FuncSet preinst_funcs;
    std::string exit_string("__GI_exit");
    std::string resolver_string("_dl_runtime_resolve_xsave");
    dt::Address resolver_addr = NULL;
    for (ph::AddrSpace::ObjMap::iterator ci = g_as->objMap().begin();
       ci != g_as->objMap().end(); ci++) {
      SpObject* obj = OBJ_CAST(ci->second);
      if (obj->name().find("libc.so") != string::npos) {
        g_parser->GetFuncsByName(obj, exit_string, false, &preinst_funcs);
      }

      // if the resolver function is not found yet, we look into this symtab's regions
      // and find the .got.plt section. We look at the 3rd entry in the table, which
      // should contain the address of the resolver function
      if (!resolver_addr) {
        sb::Symtab* tab = obj->symtab();
        std::vector<sb::Region*> regions;
        bool ret = tab->getAllRegions(regions);
        if (!ret) sp_perror("Error");
        for (std::vector<sb::Region*>::iterator reg = regions.begin(); reg != regions.end(); reg++) {
          sp_debug("Got region [%s]", (*reg)->getRegionName().c_str());
          if ((*reg)->getRegionName().compare(".got.plt") == 0) {
            dt::Address mem_offset = (*reg)->getMemOffset();
            dt::Address code_base = obj->codeBase();
            sp_debug("Mem offset is [%lx] obj base [%lx] for obj [%s]", mem_offset, code_base, obj->name().c_str());
            dt::Address* got_entry = (dt::Address*) (mem_offset + code_base + 16);  // offset to the 3rd entry
            sp_debug("dereferencing [%lx]", got_entry);
            sp_debug("resolver function addr [%lx]", *got_entry);

            if (*got_entry != NULL) {
              resolver_addr = *got_entry;
            }
          }  // end .got.plt
        }  // end region loop
      }  // end if !resolver_addr

      if (obj->name().find("ld-linux-x86-64.so") != string::npos) {
        dt::Address func_offset = resolver_addr - obj->codeBase();
        pe::CodeObject* co = obj->co();
        co->parse(func_offset, true);
        pe::CodeSource* cs = co->cs();

        set<pe::CodeRegion *> region_match;
        set<pe::Block *> blocks;
        std::vector<pe::CodeRegion *> all_regions = cs->regions();

        for (std::vector<pe::CodeRegion*>::iterator cr = all_regions.begin(); cr != all_regions.end(); cr++) {
          sp_debug("Region Low[%lx] - [%lx]", (*cr)->low(), (*cr)->high());
        }

        int cnt = cs->findRegions(func_offset, region_match);
        if (cnt == 1) {
          std::set<pe::Function*> funcs;
          co->findBlocks(*region_match.begin(), func_offset, blocks);

          pe::Block* b = *(blocks.begin());
          sp_debug("Block size [%lx], Addr [%lx]", b->size(), b->start());
          co->findFuncs(*region_match.begin(), func_offset, funcs);
          
          if (funcs.size() > 0) {
            pe::Function* res = *funcs.begin();
            sp_debug("out edges [%d]", res->callEdges().size());
            for (pe::Function::blocklist::iterator blk = res->blocks().begin(); blk != res->blocks().end(); blk++) {
              sp_debug("Block size [%ld] start [%lx]", (*blk)->size(), (*blk)->start());
            }
            sp_debug("%s", g_parser->DumpInsns((void*) resolver_addr,
						 													 78).c_str());
            if (b->parsed()) {
              sp_debug("Function is parsed");
            }
            // Instrument the found resolver function
            g_context->init_propeller()->go(FUNC_CAST(obj->getFunc(*funcs.begin())),
                                            g_context->init_entry(),
                                            g_context->init_exit());
          }
        } else {
          sp_debug("Found %d region", cnt);
        }
      }
    }

    for (FuncSet::iterator i = preinst_funcs.begin();
        i != preinst_funcs.end(); i++) {
      if (*i == NULL) continue;
      SpFunction* f = *i;
      sp_debug("Pre-instrumenting function: %s", f->name().c_str());
      g_context->init_propeller()->go(f,
                                      exit_function_payload,
                                      g_context->init_exit());
    }

    // a sleep interval to attach gdb
    if (getenv("SP_GDBSLEEP")) {
      sleep(15);
    }
  }

}
