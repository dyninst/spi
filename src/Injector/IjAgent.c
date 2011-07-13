#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
  char libname[512];
  char err[512];
} IjMsg;
#define SHMSZ sizeof(IjMsg)

__attribute__((constructor))
void ij_agent() {
  int shmid;
  key_t key = 1986;
  IjMsg* shm;
  if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  if ((shm = (IjMsg*)shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }
  void* h = dlopen(shm->libname, RTLD_NOW|RTLD_GLOBAL);
  if (!h)
    strcpy(shm->err, dlerror());
  else
    strcpy(shm->err, "ok");
}
