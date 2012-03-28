#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void foo() {
  printf("default\n");
}

void foo1() {
	printf("foo1?\n");
}

void foo2() {
	printf("foo2?\n");
}

void* t1_func(void* id) {
	long tid = (long)id;
	switch (tid) {
	case 1:	foo1(); break;
	case 2:	foo2(); break;
  default: foo(); break;
	}
}

void* t2_func(void* id) {
	long tid = (long)id;
	switch (tid) {
		case 1: foo1(); break;
    case 2: foo2(); break;
    default: foo(); break;
	}
}

typedef void* (*func_t)();

int main (int argc, char *argv[]) {
  /*
  pthread_t t[2];
  pthread_create(&t[0], NULL, t1_func, (void*)1);
  pthread_create(&t[1], NULL, t1_func, (void*)2);
  pthread_join(t[0], NULL);
  pthread_join(t[1], NULL);
  */
  
	pthread_t t[20];
  
  long i;
  for (i = 0; i < 10; i++) {
    pthread_create(&t[i], NULL, t1_func, (void*)i);
    pthread_create(&t[10+i], NULL, t2_func, (void*)i);
  }
  for (i = 0; i < 10; i++) {
    pthread_join(t[i], NULL);
    pthread_join(t[10+i], NULL);
  }
}   
