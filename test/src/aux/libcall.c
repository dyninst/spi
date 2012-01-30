#include <stdio.h>
#include <dlfcn.h>

extern void test_lib_foo();

// extern char* str;

typedef void (*foo_t)();

typedef struct {
  foo_t f;
} dummy;

int main(int argc, char *argv[]) {
	/*
	void* h = dlopen("./comp_test_agent.so", RTLD_NOW);
	if (!h) {
		printf("%s\n",dlerror());
	}
	*/
	/*
	dummy d;
	d.f = test_lib_foo;
	d.f();
	*/
	test_lib_foo();
	//printf("%s, %d\n", str, 1986);

	return 0;
}
