#include "test_case.h"

void func(int a, char* b) {
}

void func1(int a) {
}

void func2(int a, int c) {
}

int main(int argc, char** argv) {

  int a = 1986;
  char* b = "hello";
  int c = 1985; 
  func(a, b);
  func1(a);
  func2(a, c);
  return 0;
}
