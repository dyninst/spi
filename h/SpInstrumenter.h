#ifndef _SPINSTRUMENTER_H_
#define _SPINSTRUMENTER_H_

#include "SpAgentCommon.h"
#include "SpSnippet.h"
#include "SpPoint.h"

namespace sp {

	// Forward declaration
	class InstWorker;

	// ------------------------------------------------------------------- 
	// Default implementation of Instrumenter, which invokes different
  // workers to generate binary code:
  // 1. We first try to overwrite call instruction with a short jump
  // 2. If it doesn't work, we try to overwrite the call block with a
  //    short jump or a long jump, depending on the size of call block
  // 3. If it still doesn't work, we build one-hop spring board
  // 4. If it doesn't work at all, we resort to overwrite call insn w/
  //    a trap instruction
	// -------------------------------------------------------------------
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
	};

	// ------------------------------------------------------------------- 
	// A bunch of workers to generate binary code
	// -------------------------------------------------------------------
	// The base class for workers
	class InstWorker {
	public:
		// Instrument a point
		virtual bool run(SpPoint* pt) = 0;

		// Uninstrument a point
		virtual bool undo(SpPoint* pt) = 0;

		// Save code that will be modified for a point
		virtual bool save(SpPoint* pt);

		// How to install instrumentation?
		// - SP_NONE,
    // - SP_TRAP,      
    // - SP_RELOC_INSN,
    // - SP_RELOC_BLK, 
    // - SP_SPRINGBOARD
		virtual InstallMethod install_method() const = 0;

		virtual ~InstWorker() {}
	protected:
		// Install the instrumentation
		virtual bool install(SpPoint* pt) = 0;
	};

	// Overwrite call insn with a trap instruction
	class TrapWorker : public InstWorker {
	public:
    TrapWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_TRAP; }
	private:
		typedef std::map<dt::Address, SpSnippet::ptr> InstMap;
		static InstMap inst_map_;

		// For trap handler
		static void trap_handler(int sig, siginfo_t* info, void* c);
		static size_t est_blob_size();

		virtual bool install(SpPoint* pt);
	};

	// Overwrite call insn with a short jump
	class RelocCallInsnWorker : public InstWorker {
	public:
	  RelocCallInsnWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_RELOC_INSN; }
	protected:
		virtual bool install(SpPoint* pt);
	};

	// Overwrite call block with a short jump or a long jump
	class RelocCallBlockWorker : public InstWorker {
	public:
    RelocCallBlockWorker() : InstWorker() {}

		virtual bool run(SpPoint* pt);
		virtual bool undo(SpPoint* pt);
		virtual InstallMethod install_method() const { return SP_RELOC_BLK; }
	protected:
		virtual bool install(SpPoint* pt);
		
		bool install_jump_to_block(SpPoint* pt, char* jump_insn,
															 size_t insn_size);
	};

	// Generate one-hop springboard
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
