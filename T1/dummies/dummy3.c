#include <stdio.h>

int fatorial(int n) {
	if(n==1)
		return 1;
	return n*fatorial(n-1);
}

int main(void) {
	int i,j=0;
	while(j++ < 5) {
		for(i = 1; i<12; i+=2)
			fatorial(i);
		printf("dummy3 rodando...\n");
		fflush(stdout);
	}
	printf("dummy3: acabei!\n");
	fflush(stdout);
	return 0;
}