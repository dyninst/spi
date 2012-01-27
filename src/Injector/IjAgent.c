#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char libname[512];
  char err[512];
  char loaded;
  long pc;
  long sp;
  long bp;
} IjMsg;
#define SHMSZ sizeof(IjMsg)

void ij_agent() {
  int shmid;
  key_t key = 1986;
  IjMsg* shm;
  if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  if ((char*)(shm = (IjMsg*)shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }
  void* h = dlopen(shm->libname, RTLD_NOW|RTLD_GLOBAL);
  if (!h) {
    shm->loaded = 0;
    strcpy(shm->err, dlerror());
  }
  else {
    shm->loaded = 1;
    char str[512];
    sprintf(str, "Injectee [pid = %5d]: INJECTED", getpid());
    strcpy(shm->err, str);
  }
}
