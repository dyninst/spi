/* popen */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXSTRS 5

int main(void) {
  int  cntr;
  FILE *pipe_fp;
  char *strings[MAXSTRS] = { "echo", "bravo", "alpha",
			     "charlie", "delta"};

  char name[255];
  // sort, cat, od, wc, cksum, tee
  char* prog = "cat";
  sprintf(name, "/scratch/wenbin/binaries/bin/bin/%s", prog);
  /* Create one way pipe line with call to popen() */
  if (( pipe_fp = popen(name, "w")) == NULL)
    {
      perror("popen");
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

