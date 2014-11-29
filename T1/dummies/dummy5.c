#include <stdio.h>
#include <unistd.h>

int main(void) {
	int i=0, n=1;
	while(i++ < 16) {
		n=n+n;
		sleep(1);
		printf("dummy5 rodando...\n");
		fflush(stdout);
	}
	printf("dummy5: acabei!\n");
	fflush(stdout);
	return 0;
}