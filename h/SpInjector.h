#ifndef SP_INJECTOR_H_
#define SP_INJECTOR_H_

#include "SpCommon.h"
#include "Process.h"

namespace sp {

	// The Injector is a process that injects a shared library into a running
  // process's address space.
	class SpInjector {
  public:
    typedef dyn_detail::boost::shared_ptr<SpInjector> ptr;
    static ptr create(dt::PID pid);
    static void* get_shm(int id, size_t size);
    ~SpInjector();

    void inject(const char* lib_name);

    typedef struct {
      const char *libname;
      int mode;
      void* link_map;
    } dlopen_args_t;
  protected:
    dt::PID pid_;
    dt::ProcControlAPI::Process::ptr proc_;
    dt::ProcControlAPI::Thread::ptr thr_;
    StringSet dep_names_;

    SpInjector(dt::PID pid);

    dt::Address find_func(char* name);
    bool get_resolved_lib_path(const std::string &filename, StringSet &paths);
    void verify_lib_loaded(const char* libname);
    void inject_internal(const char*);
    void invoke_ijagent();
    dt::Address get_pc();
    dt::Address get_sp();
    dt::Address get_bp();

    // Platform-dependent methods. See Injector-i386.C and Injector-x86_64
    size_t get_code_tmpl_size();
    char* get_code_tmpl(dt::Address args_addr,
												dt::Address do_dlopen,
                        dt::Address code_addr);
    size_t get_ij_tmpl_size();
    char* get_ij_tmpl(dt::Address ij_addr,
                      dt::Address code_addr);
    bool is_lib_loaded(const char* libname);
    void update_frame();
	};

}

#endif /* SP_INJECTOR_H_ */
