#include <stdio.h>
#include <iostream>

namespace ns {
extern void foo1(int a);
extern void foo1(int a, int b);
}

int main(int argc, char *argv[]) {
  ns::foo1(1988);
  ns::foo1(1988, 1989);
  return 0;
}

namespace ns {

void foo1(int a, int b) {
  printf("hello, foo1(%d, %d)\n", a, b);
}

void foo1(int a) {
  std::cout << "hello, foo1 -- " <<  a << "\n";
}


int foo2(int b, int c) {
  printf("hello, foo2(%d, %d)\n", b, c);
  foo1(b);
}

class c {
 public:
  void foo3(int d) {
    std::cout << "hello, foo3(" << d << ")\n";
  }
};

}
