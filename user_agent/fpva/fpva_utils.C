#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <sys/time.h>

#include "fpva_utils.h"

namespace fpva {

std::string FpvaUtils::GetUserName(uid_t id) {
  struct passwd* p;
  if ((p = getpwuid(id)) != NULL) return p->pw_name;
  return "";
}

std::string FpvaUtils::GetGroupName(gid_t id) {
  struct group* g = getgrgid(id);
  if (g) return g->gr_name;
  return "";
}

// -------------------------------------------------------------------
// Get usecs since 1970
// -------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////

unsigned long FpvaUtils::GetUsec() {
  struct timeval tv;
  int ret = gettimeofday(&tv, NULL);
  if (ret == 0) {
    return ((tv.tv_sec - 1339166869l) * 1000000 + tv.tv_usec);
  }
  return 0;
}

}  // namespace mist
