#ifndef SP_COMMON_H_
#define SP_COMMON_H_

// Some macros for code readability
#define OVERRIDE
#define OVERLOAD

#define COMMON_EXPORT
#define INJECTOR_EXPORT
#define AGENT_EXPORT

// Some constants
const int kLenStringBuffer = 255;

// Print facility
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

// Gets file name from a full path name
#define sp_filename(path) basename((char*)path)

// Forward decalarations
namespace Dyninst {
  class MachRegister;
  
	namespace SymtabAPI {
		class Symbol;
		class Symtab;
    class AddressLookup;
	}

	namespace ParseAPI {
		class CodeObject;
		class CodeSource;
	}

  namespace PatchAPI {
    class Point;
		class PatchObject;
    class PatchFunction;
    class PatchBlock;
  }

	namespace ProcControlAPI {
	}

  namespace Stackwalker {
  }

  namespace InstructionAPI {
  }
}

// Shorten namespace
namespace dt = Dyninst;
namespace pe = Dyninst::ParseAPI;
namespace ph = Dyninst::PatchAPI;
namespace sb = Dyninst::SymtabAPI;
namespace sk = Dyninst::Stackwalker;
namespace pc = Dyninst::ProcControlAPI;
namespace in = Dyninst::InstructionAPI;

// Pointer Conversion
#define DYN_CAST(type, obj) \
	dyn_detail::boost::dynamic_pointer_cast<type>(obj)

#define FUNC_CAST(f) static_cast<SpFunction*>(f)
#define BLK_CAST(b) static_cast<SpBlock*>(b)
#define OBJ_CAST(o) static_cast<SpObject*>(o)
#define AS_CAST(a) static_cast<SpAddrSpace*>(a)

#define IJMSG_ID        1986
#define IJLIB_ID        1985
#define IJAGENT         "libijagent.so"

#endif /* SP_COMMON_H_ */
