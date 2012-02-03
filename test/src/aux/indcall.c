#include <dlfcn.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <stdio.h>

typedef void (*foo_t)();

void foo() {
	sleep(1);
}

void bar() {
}

typedef struct {
  foo_t f;
} dummy;

void hello(int a) {
	dummy t;
	t.f = bar;
	t.f();

	dummy g;
	g.f = t.f;
	g.f();

	printf("%d\n", a);
}

int main(int argc, char** argv) {
	/*
	void* h = dlopen("./comp_test_agent.so", RTLD_NOW);
	if (!h) {
		printf("%s\n",dlerror());
	}
*/
	// original
  dummy d;
  d.f = foo;
  d.f();

  d.f = bar;
  d.f();

  bar();
	// original end
	/*
	hello(1988);
	*/
  return 0;
}
