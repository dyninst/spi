#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


void foo(){
  std::cout << "In foo\n";
}

void foo2() {
  std::cout << "In foo2\n";
}

int foo_clone(void* args){
  return 0;
}

int main(){
  
  std::cout << "Hello World\n";
  foo();
  int file = open("demofile", O_RDWR);   
  close(file);
  file = open("demofile", O_RDWR);
  
  FILE* f = fopen("demofile", "r");
  if (f == NULL)
    std::cout << errno << " fopen failed\n";
  fclose(f);

  f = freopen("demofile_err", "r", stderr);
  f = fdopen(file, "r");

  chmod("demofile_mod", S_IRWXU);

  int dir = open("./", 0);
  int file2 = openat(dir, "demofile", O_RDWR);
 
  void *stack = malloc(8192);
  //pid_t pid_clone = clone(&foo_clone, (char *)stack+8192, CLONE_VM, 0);

  char *args[] = {NULL}; 
  char *envs[] = {NULL};
  char *arg = NULL;

  // execve("./calltest_fork", args, NULL);
  // pid_t pid = 99;
  
  pid_t pid = fork();

  if (pid == -1)
    std::cout << "Error forking\n";
  else if (pid == 0){
    int c = close(file);
    int f = open("demofile", O_RDWR);
    std::cout << "Testing forked proces\n";
    std::cout << "forktest below\n";

    char a0[] = "calltest_fork";
    char a1[] = "-a";
    char a2[] = "testname";
    char* argument_list[] = {a0, a1, a2, NULL};

    execv("./calltest_fork", args);
    // execve("./calltest_fork", args, NULL);
    //execvp("calltest_fork", args);
    //execl("./calltest_fork", arg, NULL);
    //execlp("calltest_fork", arg, NULL);
    //execle("calltest_fork", arg, NULL, NULL);
    //execvpe("calltest_fork", args, NULL);
    // std::cout << "forktest below\n";
    // int forktest = open("./calltest_fork", O_RDONLY);
    // std::cout << "open ret value: " << forktest << std::endl;
    // int ret = fexecve(forktest, args, envs);
    // std::cout << ret << std::endl;
  }
  else
    std::cout << "Parent end\n";
  
/*  pid = fork();
  if (pid == 0) execv("./calltest_fork", args);
  
  pid = fork();
  if (pid == 0) execve("./calltest_fork", args, NULL);

  pid = fork();
  if (pid == 0) execvp("calltest_fork", args);

  pid = fork();
  if (pid == 0) execl("./calltest_fork", arg, NULL);

  pid = fork();
  if (pid == 0) execlp("calltest_fork", arg, NULL);

  pid = fork();
  if (pid == 0) execle("calltest_fork", arg, NULL, NULL);
*/
//  pid = fork();
//  if (pid == 0) execvpe("calltest_fork", args, NULL);

  foo2();     

  chdir("../");
  chroot("../");

  chown("demofile", 0, 0);

  /*
   * open as root, de-escalate
   * 
   * close on exec?
   * fork, open files?
   * close?
   * fexecve
   * execveat
   * open, openat
   * clone2
   * exit
   * exit_group
   * 
   * capture working directory/exe path at runtime instead of in python lookup
   * chmod
   * 
   * */
  return 0;
}
