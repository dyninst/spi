/*
 * Copyright (c) 1996-2011 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SP_COMMON_H_
#define SP_COMMON_H_

#include <stdio.h>
#include <string.h>

// Some macros for code readability
#define OVERRIDE
#define OVERLOAD

#define COMMON_EXPORT __attribute__((visibility ("default")))
#define INJECTOR_EXPORT __attribute__((visibility ("default")))
#define AGENT_EXPORT __attribute__((visibility ("default")))

// Some constants
const int kLenStringBuffer = 280;

//Since condor does not like outputting stuffs to g_error_fp or g_output_fp
extern FILE* g_output_fp;
extern FILE* g_debug_fp;
extern FILE* g_error_fp;

// Print facility
#define sp_perror(...) do {\
  if (getenv("SP_DEBUG")) {   \
  		char* nodir = basename((char*)__FILE__);							\
    fprintf(stderr, "ERROR in %s [%d]: ", nodir, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    ::exit(0);						 \
  } \
  else if (getenv("SP_FDEBUG")) {               \
    char* nodir = basename((char*)__FILE__);							\
    fprintf(g_error_fp, "ERROR in %s [%d]: ", nodir, __LINE__); \
    fprintf(g_error_fp, __VA_ARGS__); \
    fprintf(g_error_fp, "\n"); \
    ::exit(0);						 \
  }					 \
	} while(0)
/*
#define sp_print(...) do {\
  fprintf(g_output_fp, __VA_ARGS__); \
  fprintf(g_output_fp, "\n"); \
  fflush(g_output_fp); \
} while(0)
*/
#define sp_print(...) do { \
  if (getenv("SP_FDEBUG")) {  \
    fprintf(g_output_fp, __VA_ARGS__);  \
    fprintf(g_output_fp, "\n");  \
  } else {  \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "\n"); \
  }  \
} while(0)

#define sp_debug(debug_type, ...) do { \
  if ((strcmp(debug_type, "injector") == 0 && getenv("SP_DEBUG_INJECTOR")) || \
      (strcmp(debug_type, "common") == 0 && getenv("SP_DEBUG_COMMON")) || \
      (strcmp(debug_type, "patchapi") == 0 && getenv("SP_DEBUG_PATCHAPI")) || \
      (strcmp(debug_type, "ipc") == 0 && getenv("SP_DEBUG_IPC")) || \
      (strcmp(debug_type, "worker") == 0 && getenv("SP_DEBUG_WORKER")) || \
      (strcmp(debug_type, "sigtrap") == 0 && getenv("SP_DEBUG_SIGTRAP")) || \
      (strcmp(debug_type, "agent") == 0 && getenv("SP_DEBUG_AGENT"))) { \
    if (getenv("SP_DEBUG")) {   \
        char* nodir = basename((char*)__FILE__);				 \
        fprintf(stderr, "%s [%d]: ", nodir, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");  \
        fflush(stderr); \
      } \
    else if (getenv("SP_FDEBUG")) {               \
        char* nodir = basename((char*)__FILE__);				 \
        fprintf(g_debug_fp, "%s [%d]: ", nodir, __LINE__); \
        fprintf(g_debug_fp, __VA_ARGS__); \
        fprintf(g_debug_fp, "\n");  \
        fflush(g_debug_fp); \
    }\
  } \
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

// Shortcut for pointer Conversion / shared pointer stuffs
#define DYN_CAST(type, obj) \
	boost::dynamic_pointer_cast<type>(obj)

#define FUNC_CAST(f) static_cast<SpFunction*>(f)
#define BLK_CAST(b) static_cast<SpBlock*>(b)
#define OBJ_CAST(o) static_cast<SpObject*>(o)
#define AS_CAST(a) static_cast<SpAddrSpace*>(a)
#define PT_CAST(a) static_cast<SpPoint*>(a)

#define SHARED_THIS(c) boost::enable_shared_from_this<c>
#define SHARED_PTR(c) boost::shared_ptr<c>

#define IJMSG_ID        1986
#define IJAGENT         "libijagent.so"

#endif  // SP_COMMON_H_
