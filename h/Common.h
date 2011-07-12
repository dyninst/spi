#ifndef SP_COMMON_H_
#define SP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#include "dyn_detail/boost/shared_ptr.hpp"

/* Print facility */
#define sp_perror(...) do {\
  fprintf(stderr, "ERROR in %s [%d]: ", __FILE__, __LINE__); \
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
      fprintf(stderr, "%30s [%5d]: ", __FILE__, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\n");  \
      fflush(stderr); \
    } \
    else ; \
} while(0)

#endif /* SP_COMMON_H_ */
