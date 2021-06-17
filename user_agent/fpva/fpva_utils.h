#ifndef _CHECKERUTILS_H_
#define _CHECKERUTILS_H_

#include "SpInc.h"

namespace fpva {

class FpvaUtils {
  public:
    static unsigned long GetUsec();
    static std::string GetUserName(uid_t id);
    static std::string GetGroupName(gid_t id);
};

}

#endif /* _CHECKERUTILS_H_ */
