#include <stdio.h>

void foo() {
	printf("In foo()\n");
}

int main() {
	printf("before foo\n");
	foo();
	printf("after foo\n");
	return 0;
}
