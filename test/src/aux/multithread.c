#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void foo1() {
	printf("foo1?\n");
}

void* t1_func(void* id) {
	long tid = (long)id;
	switch (tid) {
	case 1:	foo1(); break;
	}
}

void foo2() {
	printf("foo2?\n");
}

void* t2_func(void* id) {
	long tid = (long)id;
	if (tid) {
		foo2();
	}
}

int main (int argc, char *argv[]) {
	pthread_t t1, t2;
	t1_func(0);
	t2_func(0);
	pthread_create(&t1, NULL, t1_func, (void*)1);
	pthread_create(&t2, NULL, t2_func, (void*)1);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}   
