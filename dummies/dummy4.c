#include <stdio.h>

int main(void) {
	float var;
	int i = 0;
	while(1) {
		var = 1+2+3+4;
		var *= 0.6;
		if(++i % 10000000 == 0)
			printf("dummy4 rodando...\n");
	}
	return 0;
}