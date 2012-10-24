#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char *argv[]) {
  // printf("%d\n", getpid());
	void* h = dlopen("/usr/lib64/libz.so", RTLD_NOW);
	if (!h) {
		printf("%s\n",dlerror());
	}

	return 0;
}
