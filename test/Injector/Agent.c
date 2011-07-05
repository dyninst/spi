#include <stdio.h>

__attribute__((constructor))
void hello() {
  printf("*********************** Hello *******************************!\n");

}
