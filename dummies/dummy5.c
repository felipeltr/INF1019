#include <stdio.h>
#include <unistd.h>

int main(void) {
	int i=0, n;
	while(i++ < 8) {
		n=1+1;
		sleep(1);
		printf("dummy5 rodando...\n");
	}
	printf("dummy5: acabei!\n");
	return 0;
}