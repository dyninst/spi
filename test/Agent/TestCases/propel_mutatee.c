#include "config.h"


#include "test_case.h"
#include <dlfcn.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>

static volatile locale_t c_locale_cache;

static inline locale_t
c_locale (void)
{
  if (!c_locale_cache)
    c_locale_cache = newlocale (LC_ALL_MASK, "C", (locale_t) 0);
  return c_locale_cache;
}

double
C_STRTOD (char const *nptr, char **endptr)
{
  double r;

  locale_t locale = c_locale ();
  if (!locale)
    {
      if (endptr)
        *endptr = (char *) nptr;
      return 0; /* errno is set here */
    }

  r = strtod_l(nptr, endptr, locale);
  return r;
}

double recursion() {
  char* end;
  char* str = "899999999.99";
  return C_STRTOD(str, &end);
}

void foo() {
  printf("hello");
}
int main(int argc, char** argv) {
  void* h = dlopen("/afs/cs.wisc.edu/p/paradyn/development/wenbin/spi/test/Agent/x86_64-unknown-linux2.4/parser_agent.so", RTLD_NOW);
  if (!h) {
    fprintf(stderr, "%s", dlerror());
  }
  recursion();
}
