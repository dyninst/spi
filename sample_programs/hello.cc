#include <iostream>

void foo(){
  std::cout << "In foo\n";
}

int main(){
  
  std::cout << "Hello World\n";
  foo();
  system("condor_master");
  return 0;
}
