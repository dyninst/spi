#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  char* buf= "asfafssfaf/0asffasfsffs/0";
  printf("%d", strlen(buf));
  return 0;
}
