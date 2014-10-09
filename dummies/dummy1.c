#include <stdio.h>

int main(void) {
	float var;
	int i = 0;
	while(1) {
		var = 1+2+3+4;
		var *= 0.6;
		if(++i % 100000 == 0) {
			printf("dummy1 rodando...\n");
			fflush(stdout);
		}
		if(i > 2000000)
			return 0;
	}
	printf("dummy1: acabei!\n");
	fflush(stdout);
	return 0;
}