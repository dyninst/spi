#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  
  void* a = malloc(2);
  printf("%lx\n", a);
  free(a);
  void* b = malloc(3);
  char* d;
  free(d);
  return 0;
}
