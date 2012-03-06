#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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
	}
}

void* t2_func(void* id) {
	long tid = (long)id;
	switch (tid) {
		case 1: foo1(); break;
    case 2: foo2(); break;
	}
}

typedef void* (*func_t)();

int main (int argc, char *argv[]) {
	pthread_t t[10];
  // t1_func(0);
  
  int i;
  for (i = 0; i < 10; i++) {
    pthread_create(&t[i], NULL, t1_func, (void*)i);
  }
  for (i = 0; i < 10; i++) {
    pthread_join(t[i], NULL);
  }
}   
