#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (setuid(7132) == -1) {
    printf("failed to setuid\n");
  }

  if (setresgid(7132, 7132, 7132) == -1) {
    printf("failed to setuid\n");
  }

  return 0;
}
