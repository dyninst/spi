#ifndef SP_INJECTOR_H_
#define SP_INJECTOR_H_

#include "Process.h"

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
    static dlopen_args_t* args_; // in mutatee's address space

    static void verify_lib_loaded(const Dyninst::ProcControlAPI::Process::ptr proc,
                                  char* libname);
    static char* libname_;       // in mutator's address space

  protected:
    Dyninst::PID pid_;
    Dyninst::ProcControlAPI::Process::ptr proc_;
    Dyninst::ProcControlAPI::Thread::ptr thr_;

    Injector(Dyninst::PID pid);
    Dyninst::Address find_do_dlopen();
    static Dyninst::ProcControlAPI::Library::ptr
    find_lib(Dyninst::ProcControlAPI::Process::ptr proc, char* name);

    void verify_lib_loaded();

    /* Platform-dependent methods. See Injector-i386.C and Injector-x86_64 */
    size_t get_code_tmpl_size();
    char* get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                        Dyninst::Address code_addr);
};

}

#endif /* SP_INJECTOR_H_ */
