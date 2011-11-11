#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char* fn = getenv("TEMP_FILE");
  if (!fn) {
    printf("NULL file name\n");
    return 0;
  }
  int fd = open(fn, O_TRUNC);
  close(fd);
  return 0;
}
