#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  char* p1 = malloc(1024);
  char* p2 = malloc(1025);
  printf("p1: %lx, p2: %lx\n", p1, p2);

  free(p2);

  char* p3 = p1;
  free(p3);
  free(p1);
  return 0;
}
