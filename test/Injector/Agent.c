#include <stdio.h>
#include <dlfcn.h>

__attribute__((constructor))
void hello() {
  printf("*********************** Hello *******************************!\n");

}
