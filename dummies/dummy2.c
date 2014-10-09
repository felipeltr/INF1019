#include <stdio.h>
#include <unistd.h>

int main(void) {
	printf("dummy2 rodando...\n");
	fflush(stdout);
	sleep(1);
	printf("dummy2: acabei!\n");
	fflush(stdout);
	return 0;
}