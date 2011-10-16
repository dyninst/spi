void foo() {
}

int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < 1000000000; i++) {
    foo();
  }
  return 0;
}

