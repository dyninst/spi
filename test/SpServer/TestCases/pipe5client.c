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
  char line[MAX_LINE];
  FILE* fp = fopen("/tmp/myFIFO", "r");
  fread(line, 80, 1, fp);
  printf("line: %s\n", line);
  fclose(fp);
  return 0;
}
