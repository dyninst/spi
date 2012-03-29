#include <dirent.h>
#include <dlfcn.h>
#include <sys/types.h>

#include "PatchMgr.h"
#include "agent/parser.h"
#include "test/common/common_unittest.h"

bool
SpTestUtils::LoadManyLibs(const char* libdir,
                          const int maxlibs) {
  DIR* dir = opendir(libdir);
  if (!dir) return false;

  int count = 0;
  
  dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
    // printf("%s\n", ent->d_name);
    void* h = dlopen(ent->d_name, RTLD_NOW);
    if (h) count++;

    // Only allow up to `maxlibs` libraries loaded
    if (count > maxlibs) break;
  }

  // char show_maps[1024];
  // sprintf(show_maps, "cat /proc/%d/maps", getpid());
  // system(show_maps);
  return true;
}

sp::SpAgent::ptr
SpTestUtils::GetAgent() {
  sp::SpAgent::ptr agent = sp::SpAgent::Create();
  assert(agent);
  agent->EnableParseOnly(true);
  agent->Go();
  return agent;
}
