#ifndef _SPAGENTCOMMON_H_
#define _SPAGENTCOMMON_H_

#include "SpCommon.h"

/* Dyninst headers for Agent */
#include "frame.h"
#include "Point.h"
#include "walker.h"
#include "Visitor.h"
#include "PatchMgr.h"
#include "PatchCFG.h"
#include "AddrSpace.h"
#include "Immediate.h"
#include "CodeObject.h"
#include "PatchObject.h"
#include "Instruction.h"
#include "Instrumenter.h"
#include "BinaryFunction.h"

/* Shorten namespace */
namespace dt = Dyninst;
namespace pe = Dyninst::ParseAPI;
namespace ph = Dyninst::PatchAPI;
namespace sb = Dyninst::SymtabAPI;
namespace in = Dyninst::InstructionAPI;
namespace sk = Dyninst::Stackwalker;

#endif /* _SPAGENTCOMMON_H_ */
