#ifndef SP_INJECTOR_H_
#define SP_INJECTOR_H_

#include "SpCommon.h"
#include "Process.h"

namespace sp {

/* The Injector is a process that injects a shared library into a running
   process's address space. */
class SpInjector {
  public:
    typedef dyn_detail::boost::shared_ptr<SpInjector> ptr;
    static ptr create(Dyninst::PID pid);
    ~SpInjector();

    void inject(const char* lib_name);
    void* get_shm(int id, size_t size);

    typedef struct {
      const char *libname;
      int mode;
      void* link_map;
    } dlopen_args_t;
  protected:
    Dyninst::PID pid_;
    Dyninst::ProcControlAPI::Process::ptr proc_;
    Dyninst::ProcControlAPI::Thread::ptr thr_;
    typedef std::set<std::string> DepNames;
    DepNames dep_names_;

    SpInjector(Dyninst::PID pid);

    Dyninst::Address find_func(char* name);
    bool get_resolved_lib_path(const std::string &filename, DepNames &paths);
    void verify_lib_loaded(const char* libname);
    void inject_internal(const char*);
    void invoke_ijagent();
    Dyninst::Address get_pc();

    /* Platform-dependent methods. See Injector-i386.C and Injector-x86_64 */
    size_t get_code_tmpl_size();
    char* get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                        Dyninst::Address code_addr);
    size_t get_ij_tmpl_size();
    char* get_ij_tmpl(Dyninst::Address ij_addr,
                      Dyninst::Address /*code_addr*/);
    bool is_lib_loaded(const char* libname);
    void update_pc();

    // Procedures to assist self-propelled instrumentation
    void identify_original_libs();
};

}

#endif /* SP_INJECTOR_H_ */
