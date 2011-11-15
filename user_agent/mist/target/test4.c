#include <stdio.h>
#include <pthread.h>

void* f1(void* arg) {
  printf("f1: i'm in %d\n", pthread_self());
  return 0;
}


void foo2() {
  pthread_t t1;
  printf("create %d @ %lx\n", t1, &t1);
  pthread_create(&t1, NULL, f1, NULL);
  printf("create %d @ %lx\n", t1, &t1);
}

void foo1()  {
  foo2();
}

int main(int argc, char *argv[]) {
  foo1();
  pthread_t t2;
  pthread_create(&t2, NULL, f1, NULL);
  pthread_join(t2, NULL);
  return 0;
}
