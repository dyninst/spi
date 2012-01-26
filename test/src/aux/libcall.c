#include <stdio.h>
#include <dlfcn.h>

extern void test_lib_foo();

// extern char* str;

int main(int argc, char *argv[]) {
	/*
	void* h = dlopen("./comp_test_agent.so", RTLD_NOW);
	if (!h) {
		printf("%s\n",dlerror());
	}
	*/
	test_lib_foo();
	//printf("%s, %d\n", str, 1986);

	return 0;
}
