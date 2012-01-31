#ifndef SP_COMMON_H_
#define SP_COMMON_H_

/* C headers */
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <ucontext.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <sys/ucontext.h>

/* C++ headers */
#include <set>
#include <map>
#include <list>
#include <stack>
#include <vector>

/* Dyninst headers */
#include "signal.h"
#include "Symbol.h"
#include "Symtab.h"
#include "Function.h"
#include "AddrLookup.h"
#include "dyn_detail/boost/shared_ptr.hpp"

/* Print facility */
#define sp_perror(...) do {\
		char* nodir = basename((char*)__FILE__);							\
  fprintf(stderr, "ERROR in %s [%d]: ", nodir, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  ::exit(0);						 \
	} while(0)

#define sp_print(...) do {\
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  fflush(stderr); \
} while(0)

#define sp_debug(...) do { \
  if (getenv("SP_DEBUG")) {   \
  		char* nodir = basename((char*)__FILE__);				 \
      fprintf(stderr, "%s [%d]: ", nodir, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\n");  \
      fflush(stderr); \
    } \
    else ; \
} while(0)


#define sp_filename(path) basename((char*)path)

namespace Dyninst {
	namespace SymtabAPI {
		class Symbol;
	}

	namespace ParseAPI {
		class CodeObject;
		class CodeSource;
	}

  namespace PatchAPI {
    class Point;
		class PatchObject;
    class PatchFunction;
  }

	namespace ProcControlAPI {
	}
}

// Shorten namespace
namespace dt = Dyninst;
namespace pe = Dyninst::ParseAPI;
namespace ph = Dyninst::PatchAPI;
namespace sb = Dyninst::SymtabAPI;
namespace pc = Dyninst::ProcControlAPI;

namespace sp {
	typedef std::set<pid_t> PidSet;
	typedef std::set<std::string> StringSet;

	typedef std::vector<Dyninst::PatchAPI::Point*> Points;
	typedef std::set<Dyninst::PatchAPI::PatchFunction*> FuncSet;
  typedef std::vector<Dyninst::ParseAPI::CodeObject*> CodeObjects;
	typedef std::vector<Dyninst::SymtabAPI::Symbol*> Symbols;
  typedef std::vector<Dyninst::PatchAPI::PatchObject*> PatchObjects;
  typedef std::vector<Dyninst::ParseAPI::CodeSource*> CodeSources;

  typedef std::map<std::string, Dyninst::PatchAPI::PatchFunction*> RealFuncMap;
}

#define DYN_CAST(type, obj) dyn_detail::boost::dynamic_pointer_cast<type>(obj)

#define IJMSG_ID        1986
#define IJLIB_ID        1985
#define IJAGENT         "libijagent.so"

#endif /* SP_COMMON_H_ */
