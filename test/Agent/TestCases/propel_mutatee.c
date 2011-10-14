#include "test_case.h"
#include <dlfcn.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>
  
#include <stdio.h>
extern int atoi2(const char *str, int n)
{
  if (str == 0 || *str == 0)
    return n;
  return atoi2(str+1, n*10 + *str-'0');
}
int main(int argc, char **argv)
{
  int i;
  for (i = 1; i != argc; ++i)
    printf("%s -> %d\n", argv[i], atoi2(argv[i], 0));
  return 0;
}
