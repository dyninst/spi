/* popen */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#define MAXSTRS 5

int main(void) {
  int  cntr;
  FILE *pipe_fp;
  const char *strings[MAXSTRS] = { "echo", "bravo", "alpha",
                                   "charlie", "delta"};

  /* Create one way pipe line with call to popen() */
  if ((pipe_fp = popen("test_mutatee/od.exe", "w")) == NULL) {
    perror("popen");
    exit(1);
  }

  /* Manually load the agent for testing */
  char path[1024];
  if (getenv("SP_AGENT_DIR")) {
    snprintf(path, 1024, "%s/ipc_test_agent.so", getenv("SP_AGENT_DIR"));
  } else {
    snprintf(path, 1024, "./ipc_test_agent.so");
  }
  void* h = dlopen(path, RTLD_NOW|RTLD_GLOBAL);
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

