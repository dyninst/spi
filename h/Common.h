#ifndef SP_COMMON_H_
#define SP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#include "dyn_detail/boost/shared_ptr.hpp"

/* Print facility */
#define sp_perror(...) do {\
  char* nodir = strrchr(__FILE__, '/') + 1; \
  fprintf(stderr, "ERROR in %10s [%5d]: ", nodir, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  exit(0);\
} while(0)

#define sp_print(...) do {\
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  fflush(stderr); \
} while(0)

#define sp_debug(...) do { \
  if (getenv("SP_DEBUG")) {   \
      char* nodir = strrchr(__FILE__, '/') + 1;   \
      fprintf(stderr, "%10s [%5d]: ", nodir, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\n");  \
      fflush(stderr); \
    } \
    else ; \
} while(0)

#endif /* SP_COMMON_H_ */
