#include <stdio.h>
#include <unistd.h>

int main(void) {
	printf("dummy7 rodando...\n");
	fflush(stdout);
	sleep(7);
	printf("dummy7: acabei!\n");
	fflush(stdout);
	return 0;
}