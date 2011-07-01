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

  protected:
    Dyninst::PID pid_;
    Dyninst::ProcControlAPI::Process::ptr proc_;

    typedef struct {
      const char *libname;
      int mode;
      void* link_map;
    } dlopen_args_t;

    Injector(Dyninst::PID pid);

    /* Platform-dependent methods. See Injector-i386.C and Injector-x86_64 */
    void save_pc();
    Dyninst::Address find_do_dlopen();
    size_t get_code_tmpl_size();
    char* get_code_tmpl(Dyninst::Address args_addr, Dyninst::Address do_dlopen,
                        Dyninst::Address code_addr);
};

}

#endif /* SP_INJECTOR_H_ */
