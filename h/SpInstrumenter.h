#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "SpAgentCommon.h"
#include "SpSnippet.h"
#include "SpPoint.h"

namespace sp {

	class InstWorker {
	public:
		// Instrument a point
		virtual bool run(SpPoint* pt) = 0;

		// Uninstrument a point
		virtual bool undo(SpPoint* pt) = 0;

		// Save code that will be modified for a point
		virtual bool save(SpPoint* pt) = 0;

		// How to install instrumentation?
		// - SP_NONE,
    // - SP_TRAP,      
    // - SP_RELOC_INSN,
    // - SP_RELOC_BLK, 
    // - SP_SPRINGBOARD
		virtual InstallMethod install_method() const = 0;
	protected:
		// Install the instrumentation
		virtual bool install(SpPoint* pt) = 0;
	};


	class SpInstrumenter : public ph::Instrumenter {
  public:
    static SpInstrumenter* create(ph::AddrSpace* as);

		virtual bool run();
		virtual bool undo();

  protected:
		typedef std::vector<InstWorker*> InstWorkers;
		InstWorkers workers_;

    SpInstrumenter(ph::AddrSpace*);
		~SpInstrumenter();

#if 0
    bool install_direct(SpPoint* point,
                        char*      blob,
                        size_t     blob_size);

    bool install_indirect(SpPoint*         point, 
                          sp::SpSnippet::ptr snip,
                          bool               jump_abs,
                          dt::Address        ret_addr);

    bool install_jump(ph::PatchBlock*     blk,
                      char*               insn,
                      size_t              insn_size,
                      sp::SpSnippet::ptr  snip, 
                      dt::Address         ret_addr);

    bool install_spring(ph::PatchBlock*    callblk,
                        sp::SpSnippet::ptr snip,
                        dt::Address        ret_addr);

    bool install_trap(SpPoint* point,
                      char*      blob,
                      size_t     blob_size);

    static void trap_handler(int        sig,
                             siginfo_t* info,
                             void*      c);
#endif
	};

	class TrapWorker : public InstWorker {
	public:
    TrapWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual bool save(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_TRAP; }
	private:
		typedef std::map<dt::Address, SpSnippet::ptr> InstMap;
		static InstMap inst_map_;

		// For trap handler
		static void trap_handler(int sig, siginfo_t* info, void* c);

		virtual bool install(SpPoint* pt);
	};

	class RelocCallInsnWorker : public InstWorker {
	public:
	  RelocCallInsnWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual bool save(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_RELOC_INSN; }
	protected:
		virtual bool install(SpPoint* pt);
	};

	class RelocCallBlockWorker : public InstWorker {
	public:
    RelocCallBlockWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual bool save(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_RELOC_BLK; }
	protected:
		virtual bool install(SpPoint* pt);
	};

	class SpringboardWorker : public InstWorker {
	public:
    SpringboardWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual bool save(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_SPRINGBOARD; }
	protected:
		virtual bool install(SpPoint* pt);
	};

}

#endif /* _SPINSTRUMENTER_H_ */
