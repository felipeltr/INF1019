#include <stdio.h>
#include <unistd.h>

int fatorial(int n) {
	if(n==1)
		return 1;
	return n*fatorial(n-1);
}

int main(void) {
	int i,j=0;
	while(j++ < 10) {
		for(i = 1; i<20; i+=2)
			fatorial(i);
		sleep(1);
		printf("dummy6 rodando...\n");
		fflush(stdout);
	}
	printf("dummy6: acabei!\n");
	fflush(stdout);
	return 0;
}