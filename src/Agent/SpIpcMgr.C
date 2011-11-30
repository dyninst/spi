#include "SpIpcMgr.h"
#include "SpInc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace sp {

SpIpcMgr::SpIpcMgr() {

#define CAN_WORK_ID 1987
#define CAN_WORK_SIZE 32768
  int shmid;
  if ((shmid = shmget(CAN_WORK_ID, CAN_WORK_SIZE, IPC_CREAT | 0666)) < 0) {
    sp_perror("ERROR: cannot create shared memory with id %d", CAN_WORK_ID);
  }

  char* shm = NULL;
  if ((long)(shm = (char*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("ERROR: cannot get shared memory");
  }
  can_work_ = &shm[getpid()];
  *can_work_ = 1;
}

bool SpIpcMgr::is_pipe(int fd) {
  struct stat s;
  if (fstat(fd, &s) == -1) return false;
  if (S_ISFIFO(s.st_mode)) {
    return true;
  }
  return false;
}

char SpIpcMgr::can_work() {
  return *can_work_;
}

void SpIpcMgr::set_work(char b, int pid) {

#define CAN_WORK_ID 1987
#define CAN_WORK_SIZE 32768
  int shmid;
  if ((shmid = shmget(CAN_WORK_ID, CAN_WORK_SIZE, IPC_CREAT | 0666)) < 0) {
    sp_perror("ERROR: cannot create shared memory with id %d", CAN_WORK_ID);
  }

  char* shm = NULL;
  if ((long)(shm = (char*)shmat(shmid, NULL, 0)) == (long)-1) {
    sp_perror("ERROR: cannot get shared memory");
  }
  can_work_ = &shm[pid];
  *can_work_ = b;
}

}
