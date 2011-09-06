#ifndef SP_TEST_CASE_H_
#define SP_TEST_CASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <dlfcn.h>

#define dprint(...) do { \
  if (getenv("SP_DEBUG")) { \
    fprintf(stderr, "%10s [%5d]: ", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");  \
    fflush(stderr); \
  }\
} while(0)

#endif /* SP_TEST_CASE_H_ */
