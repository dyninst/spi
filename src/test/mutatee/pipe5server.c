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

  int c;
  while ((c = fgetc(fp)) != EOF) {
    printf("%c", c);
  }

  /*
  fread(line, MAX_LINE, 1, fp);
  printf("line: %s\n", line);
  */
  // fprintf(stderr, "pid: %d, fd: %d", getpid(), fileno(fp));
  //  while(1);
  fclose(fp);
  return 0;
}
