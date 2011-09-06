#include "test_case.h"

int main(int argc, char** argv) {
  dprint("MUTATEE: enter event mutatee");
  int count = 0;
  while (count < 10) {
    count++;
    printf("count: %d\n", count);
  }
}
