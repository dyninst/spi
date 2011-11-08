#include <stdio.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

  FILE* fp1 = fopen("../files/test1_1", "r");
  char* fname = "../files/test1_2";
  int fp2 = open(fname, O_RDONLY);
  close(fp2);
  // fclose(fp1);
  return 0;
}
