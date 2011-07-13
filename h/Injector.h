#ifndef SP_INJECTOR_H_
#define SP_INJECTOR_H_

#include "Process.h"
#include <list>

namespace sp {

/* The Injector is a process that injects a shared library into a running
   process's address space. */
class Injector {
  public:
    typedef dyn_detail::boost::shared_ptr<Injector> ptr;
    static ptr create(Dyninst::PID pid);

    void inject(const char* lib_name);

    /* validate the result of library loading */
    static void verify_ret(const Dyninst::ProcControlAPI::Process::ptr proc,
                           Dyninst::Address args);
    typedef struct {
      const char *libname;
      int mode;
      void* link_map;
    } dlopen_args_t;
    static Dyninst::Address args_;
    static void verify_lib_loaded(const Dyninst::ProcControlAPI::Process::ptr proc,
                                  char* libname);

  protected:
    Dyninst::PID pid_;
    Dyninst::ProcControlAPI::Process::ptr proc_;
    Dyninst::ProcControlAPI::Thread::ptr thr_;
    typedef std::set<std::string> DepNames;
    DepNames dep_names_;

    Injector(Dyninst::PID pid);
    Dyninst::Address find_do_dlopen();
    bool get_resolved_lib_path(const std::string &filename, DepNames &paths);

    void verify_lib_loaded();

    void inject_internal(const char*);
    bool check_all_dependencies(const char* lib_name, DepNames& unresolved_libs);

    /* Platform-dependent methods. See Injector-i386.C and Injector-x86_64 */
    size_t get_code_tmpl_size();
    char* get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                        Dyninst::Address code_addr);
};

}

#endif /* SP_INJECTOR_H_ */
