#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_LINE 80

int main(int argc, char** argv) {

  const char* line = "hello, world!\n";
  FILE* fp = fopen("/tmp/myFIFO", "w");
  fwrite(line, strlen(line)+1, 1, fp);
  //fprintf(stderr, "pid: %d, fd: %d", getpid(), fileno(fp));
  //while(1);
  fclose(fp);

  return 0;
}
