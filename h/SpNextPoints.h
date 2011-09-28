#ifndef _SPNEXTPOINTS_H_
#define _SPNEXTPOINTS_H_

#include "PatchCFG.h"
#include "SpCommon.h"

namespace sp {
class SpContext;
void CalleePoints(Dyninst::PatchAPI::PatchFunction*,
                  SpContext*, Points&);

}

#endif /* _SPNEXTPOINTS_H_ */
