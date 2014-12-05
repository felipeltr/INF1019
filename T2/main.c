#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "mmu.h"


// Params
#define WSL 4
#define FRAMES 16

#define NTHREAD 16
#define PAGES_PER_THREAD 8

pthread_t thread[NTHREAD];

void *thread_main(void* arg) {
	int i;
	mmu_addr_t firstPage;
	for(i=0;i<PAGES_PER_THREAD;i++) {
		printf("\n\n>> tid %d solicitando pagina\n",(int)pthread_self()); fflush(stdout);
		if(i==0)
			firstPage = requestPage(clock());
		else
			requestPage(clock());
		printMemory();
		printVirtualPageTable();
		sleep(2);
	}
	printf(">>> final da tid %d. acessando valor do end virtual %d: %d\n",(int)pthread_self(), (int)firstPage, getPageContent(firstPage));
	fflush(stdout);
	freeMySpaces();
	return NULL;
}

int main (void) {
	int i;

	initMMU(WSL,FRAMES);

	for(i=0;i<NTHREAD;i++) {
		pthread_create(&thread[i],NULL,thread_main,NULL);
		sleep(5);
	}
	for(i=0;i<NTHREAD;i++) {
		pthread_join(thread[i],NULL);
	}

	destroyMMU();

	return 0;
}


