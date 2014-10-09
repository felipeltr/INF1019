#include <stdio.h>

int fatorial(int n) {
	if(n==1)
		return 1;
	return n*fatorial(n-1);
}

int main(void) {
	int i;
	while(1) {
		for(i = 1; i<20; i+=2)
			fatorial(i);
		//printf("dummy6 rodando...\n");
	}
	return 0;
}