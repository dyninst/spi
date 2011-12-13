/* popen */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#define MAXSTRS 5

int main(void) {
  int  cntr;
  FILE *pipe_fp;
  char *strings[MAXSTRS] = { "echo", "bravo", "alpha",
			     "charlie", "delta"};

  /* Create one way pipe line with call to popen() */
  if (( pipe_fp = popen("/scratch/wenbin/binaries/bin/bin/sort", "w")) == NULL)
    {
      perror("popen");
      exit(1);
    }

    /* Manually load the agent for testing */
    void* h = dlopen("./TestAgent.so", RTLD_NOW|RTLD_GLOBAL);
    if (!h) {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
    }

  /* Processing loop */

  for(cntr=0; cntr<MAXSTRS; cntr++) {
    fputs(strings[cntr], pipe_fp);
    fputc('\n', pipe_fp);
  }

  /* Close the pipe */
  pclose(pipe_fp);
        
  return(0);
}

