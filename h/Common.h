#ifndef SP_COMMON_H_
#define SP_COMMON_H_

#include "dyn_detail/boost/shared_ptr.hpp"

extern bool sp_debug_flag;

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
    if (sp_debug_flag) {\
      fprintf(stderr, "%30s [%5d]: ", __FILE__, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\n");  \
      fflush(stderr); \
    } \
    else ; \
} while(0)

/* Set the environment variable SP_DEBUG to enable debug output */
struct DebugConfig{
  DebugConfig();
};
extern DebugConfig debug_config;

#endif /* SP_COMMON_H_ */
