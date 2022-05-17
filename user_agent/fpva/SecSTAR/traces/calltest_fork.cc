#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void foo3(){
  std::cout << "In forked foo\n";
}

int foo4() {
  int i = 0;
  std::cout << i << std::endl;
}
int main(int argc, char **argv, char **envp){
  for (char **env = envp; *env != 0; env++) {
    char *thisEnv = *env;
    printf("%s\n", thisEnv);
  }
  std::cout << "Hello Forked Process\n";
  std::cout << "What\n";
  foo4();
  std::cout << "Is this still working?" << std::endl;
  foo3();
  std::cout << "Goodbye Forked Process\n";
  
  return 0;
}
