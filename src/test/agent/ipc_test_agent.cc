#include "SpInc.h"
#include <sys/resource.h>

using namespace Dyninst;
using namespace PatchAPI;
using namespace sp;

//int level = 0;

void* test_entry(SpPoint* pt) {
  SpFunction* f = Callee(pt);
  if (!f) return NULL;

	if (IsIpcWrite(pt)) {
    /*
		sp_print("Write: %s @ pid=%d w/ addr %lx",
            f->name().c_str(), getpid(), f->addr());
    */
		fprintf(stderr, "Write: %s @ pid=%d w/ addr %lx\n",
            f->name().c_str(), getpid(), f->addr());
	}
	else if (IsIpcRead(pt)) {
    /*
		sp_print("Read: %s @ pid=%d w/ addr %lx",
             f->name().c_str(), getpid(), f->addr());
    */
		fprintf(stderr, "Read: %s @ pid=%d w/ addr %lx\n",
             f->name().c_str(), getpid(), f->addr());
	}
  /*
  if (!pt->tailcall()) {
    for (int i = 0; i < level; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s @ pid=%d\n", f->name().c_str(), getpid());
    level++;
  }
  */
  sp::Propel(pt);
  return NULL;
}

void test_exit(sp::PointHandle* handle) {
  SpFunction* f = handle->GetCallee();
  if (!f) return;
  // level--;
  
	if (IsIpcWrite(handle->GetPoint())) {
		long size = handle->ReturnValue();
		// sp_print("Write size: %ld @ pid=%d", size, getpid());
		fprintf(stderr, "Write size: %ld @ pid=%d\n", size, getpid());
	} else if (IsIpcRead(handle->GetPoint())) {
		long size = handle->ReturnValue();
		// sp_print("Read size: %ld @ pid=%d", size, getpid());
		fprintf(stderr, "Read size: %ld @ pid=%d\n", size, getpid());
	}
}

AGENT_INIT
void MyAgent() {
  // fprintf(stderr, "AGENT LOADED!\n");
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  StringSet libs_not_to_inst {"linux-vdso.so",
                                "libdl.so",
                                "libresolv.so",
                                "librt.so",
                                "libcrypto.so",
                                "libstdc++.so",
                                "libm.so",
                                "libgomp.so",
                                "libpthread.so",
                                "libc.so",
                                "ld-linux-x86-64.so"};
  agent->SetLibrariesNotToInstrument(libs_not_to_inst);
  
  agent->SetInitEntry("test_entry");
  agent->SetInitExit("test_exit");
  agent->EnableIpc(true);
  agent->Go();
}

__attribute__((destructor))
void DumpOutput() {
}

