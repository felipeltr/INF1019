#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#include "mmu.h"

#define HARD_DISK_SIZE 256

/* tipos internos da MMU */

typedef struct tthreadPage {
	mmu_addr_t virtualAddr; // endereco virtual
	int lastUse;
	int onMemory; // se está na memoria
	int addr; // endereco na memoria ('active' da MMU_head) ou no disco rigido ('swappedOut' da MMU_head)
	struct tthreadPage *next;
} PageNode;

typedef struct tthread {
	pthread_t tid; // thread id
	int nPages; // contador de paginas, tanto em memoria quanto em disco
	int pagesOnMemory; // contador de paginas dessa thread atualmente na memoria
	PageNode* pageList;
	struct tthread *next;
} ThreadNode;

typedef struct tlist {
	int wsl;
	int nFrames;
	int* active; // memoria
	int* swappedOut; // disco rigido
	int framesUsed; // contador de frames em uso
	ThreadNode** threadMap; // vetor que mapeia a thread que possui cada frame na memoria;
	ThreadNode** threadMapDisk; // vetor que mapeia a thread que possui cada frame no disco;
	ThreadNode* tList; // lista de threads, onde cada elemento possui lista de paginas virtuais daquela thread
} MMU_thead;

/* cabeca encapsulada da MMU */
static MMU_thead *mmu = NULL;

/* mutex para exclusao mutua nas operacoes */
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/* metodos */

void initMMU(int wsl, int nFrames) {
	int i;
	if(mmu != NULL) return;
	mmu = (MMU_thead*) malloc(sizeof(MMU_thead));
	mmu->wsl = wsl;
	mmu->nFrames = nFrames;
	mmu->active = (int*) malloc(nFrames * sizeof(int));
	mmu->swappedOut = (int*) malloc(HARD_DISK_SIZE * sizeof(int));
	mmu->tList = NULL;
	mmu->framesUsed = 0;
	mmu->threadMap = (ThreadNode**) malloc(nFrames * sizeof(ThreadNode*));
	for(i=0;i<nFrames;i++)
		mmu->threadMap[i] = NULL;
	mmu->threadMapDisk = (ThreadNode**) malloc(HARD_DISK_SIZE * sizeof(ThreadNode*));
	for(i=0;i<HARD_DISK_SIZE;i++)
		mmu->threadMapDisk[i] = NULL;
}

static void appendThreadNode( ThreadNode* tNode) {
	ThreadNode* current = mmu->tList;
	if(current == NULL) {
		mmu->tList = tNode;
		return;
	}

	while(current != NULL) {
		if(current->next == NULL) {
			current->next = tNode;
			return;
		}

		current = current->next;
	}
}

static ThreadNode* createThreadNode(void) {
	ThreadNode* node;
	node = (ThreadNode*) malloc(sizeof(ThreadNode));
	node->tid = pthread_self();
	node->nPages = 0;
	node->pagesOnMemory = 0;
	node->pageList = NULL;
	node->next = NULL;

	return node;
}

static void appendPageNode( ThreadNode* tNode, PageNode* pNode) {
	PageNode* current = tNode->pageList;
	if(current == NULL) {
		tNode->pageList = pNode;
		return;
	}

	while(current != NULL) {
		if(current->next == NULL) {
			current->next = pNode;
			return;
		}

		current = current->next;
	}
}

static PageNode* createPageNode(void) {
	PageNode* node;
	node = (PageNode*) malloc(sizeof(ThreadNode));
	node->next = NULL;

	return node;
}

static ThreadNode* findThreadNodeByTid( pthread_t tid ) {
	ThreadNode* current = mmu->tList;
	while(current != NULL) {
		if(current->tid == tid)
			break;
		current = current->next;
	}
	return current;
}

static int getNextEmptySlotOnMemory(void) {
	int i;
	for(i=0;i<mmu->nFrames;i++)
		if(mmu->threadMap[i] == NULL)
			return i;
	return -1;
}

static int getNextEmptySlotOnDisk(void) {
	int i;
	for(i=0;i<HARD_DISK_SIZE;i++)
		if(mmu->threadMapDisk[i] == NULL)
			return i;
	return -1;
}

static int swapOutPage(void) { // MUST BE CALLED INSIDE CRITICAL REGION
	ThreadNode* currThread = mmu->tList;
	PageNode *currPage, *pageToBeSwappedOut;
	int addrOnMemory;
	clock_t t = 99999*CLOCKS_PER_SEC; // valor muito alto "nulo", para comparacao
	while(currThread != NULL) {
		currPage = currThread->pageList;
		while(currPage != NULL) {
			if(currPage->onMemory == 1 && currPage->lastUse < t) {
				pageToBeSwappedOut = currPage;
				t = pageToBeSwappedOut->lastUse;
			}

			currPage = currPage->next;
		}
		currThread = currThread->next;
	}
	pageToBeSwappedOut->onMemory = 0;
	addrOnMemory = pageToBeSwappedOut->addr;
	pageToBeSwappedOut->addr = getNextEmptySlotOnDisk();
	(mmu->framesUsed)--;
	(mmu->threadMap[addrOnMemory]->pagesOnMemory)--;
	mmu->threadMap[addrOnMemory] = NULL;
	mmu->threadMapDisk[pageToBeSwappedOut->addr] = currThread;
	mmu->swappedOut[pageToBeSwappedOut->addr] = mmu->active[addrOnMemory];

	printf(">> Memoria atingiu limite. swap out em %d. conteudo movido para %d no disco\n", addrOnMemory, pageToBeSwappedOut->addr); fflush(stdout);

	return addrOnMemory;
}

static int swapOutPageOfThread( pthread_t tid ) { // MUST BE CALLED INSIDE CRITICAL REGION
	ThreadNode* currThread = findThreadNodeByTid( pthread_self() );
	PageNode *currPage, *pageToBeSwappedOut;
	int addrOnMemory;
	clock_t t = 99999*CLOCKS_PER_SEC; // valor muito alto "nulo", para comparacao

	currPage = currThread->pageList;
	while(currPage != NULL) {
		if(currPage->onMemory == 1 && currPage->lastUse < t) {
			pageToBeSwappedOut = currPage;
			t = pageToBeSwappedOut->lastUse;
		}

		currPage = currPage->next;
	}

	pageToBeSwappedOut->onMemory = 0;
	addrOnMemory = pageToBeSwappedOut->addr;
	pageToBeSwappedOut->addr = getNextEmptySlotOnDisk();
	(mmu->framesUsed)--;
	(mmu->threadMap[addrOnMemory]->pagesOnMemory)--;
	mmu->threadMap[addrOnMemory] = NULL;
	mmu->threadMapDisk[pageToBeSwappedOut->addr] = currThread;
	mmu->swappedOut[pageToBeSwappedOut->addr] = mmu->active[addrOnMemory];

	printf(">> WSL atingiu limite. swap out em %d. conteudo movido para %d no disco\n", addrOnMemory, pageToBeSwappedOut->addr); fflush(stdout);
	
	return addrOnMemory;
}

int getPageContent(mmu_addr_t virtualAddr) {
	ThreadNode* tNode;
	PageNode* pNode;
	int value, addrToUse;
	pthread_mutex_lock(&mut);
	tNode = findThreadNodeByTid( pthread_self() );

	pNode = tNode->pageList;
	while(pNode != NULL) {
		if(pNode->virtualAddr == virtualAddr)
			break;

		pNode = pNode->next;
	}
	if(pNode == NULL) {
		printf(">>>> Acesso invalido!\n"); fflush(stdout);
		return 0;
	}

	if(pNode->onMemory) {
		value = mmu->active[pNode->addr];
		pthread_mutex_unlock(&mut);
		return value;
	}
	 // neste caso, swap out deve ser feito
	if(tNode->pagesOnMemory == mmu->wsl)
		addrToUse = swapOutPageOfThread(pthread_self());
	else if(mmu->framesUsed == mmu->nFrames)
		addrToUse = swapOutPage();
	else
		addrToUse = getNextEmptySlotOnMemory();
	

	pNode->lastUse = clock();

	(tNode->pagesOnMemory)++;

	mmu->active[addrToUse] = mmu->swappedOut[pNode->addr];
	mmu->threadMap[addrToUse] = tNode;
	mmu->threadMapDisk[pNode->addr] = NULL;
	(mmu->framesUsed)++;

	pNode->onMemory = 1;
	pNode->addr = addrToUse;

	value = mmu->active[addrToUse]; // simula copia da memoria (pois isto nao pode ser feito fora da RC)
	pthread_mutex_unlock(&mut);

	return value;

}

mmu_addr_t requestPage(int value) {
	ThreadNode* tNode;
	PageNode* pNode;
	int addrToUse;
	pthread_mutex_lock(&mut);
	tNode = findThreadNodeByTid( pthread_self() );
	if(tNode == NULL) {
		tNode = createThreadNode();
		appendThreadNode(tNode);
	}

	if(tNode->pagesOnMemory == mmu->wsl)
		addrToUse = swapOutPageOfThread(pthread_self());
	else if(mmu->framesUsed == mmu->nFrames)
		addrToUse = swapOutPage();
	else
		addrToUse = getNextEmptySlotOnMemory();

	pNode = createPageNode();
	appendPageNode(tNode,pNode);

	pNode->lastUse = clock();
	pNode->onMemory = 1;
	pNode->addr = addrToUse;
	pNode->virtualAddr = tNode->nPages;

	(tNode->pagesOnMemory)++;
	(tNode->nPages)++;

	mmu->active[addrToUse] = value;
	mmu->threadMap[addrToUse] = tNode;
	(mmu->framesUsed)++;

	pthread_mutex_unlock(&mut);

	return pNode->virtualAddr;
}

void printMemory(void) {
	int i;
	pthread_mutex_lock(&mut);
	printf("ADDR\tTID\t\tVALUE\n"); fflush(stdout);
	for(i=0;i<mmu->nFrames;i++) {
		printf("%d\t", i); fflush(stdout);
		if(mmu->threadMap[i]) {
			printf("%010d\t%d\n",(int)mmu->threadMap[i]->tid, mmu->active[i]);  fflush(stdout);
		} else {
			printf("- frame vazio -\n");  fflush(stdout);
		}
	}
	pthread_mutex_unlock(&mut);
}

void printVirtualPageTable(void) {
	ThreadNode* tNode;
	PageNode* pNode;
	char yes[4] = "yes", no[3] = "no";
	char disk[5] = "disk", memory[7] = "memory";
	pthread_mutex_lock(&mut);
	tNode = findThreadNodeByTid( pthread_self() );
	printf("Virtual Page Table for tid %d\n",(int)pthread_self());

	pNode = tNode->pageList;
	while(pNode != NULL) {
		printf("virtual addr: %d - on memory: %s - addr on %s: %d - last time used: %d\n",
			pNode->virtualAddr,(pNode->onMemory) ? yes : no, (pNode->onMemory) ? memory : disk,
			pNode->addr, pNode->lastUse); fflush(stdout);		

		pNode = pNode->next;
	}
	pthread_mutex_unlock(&mut);
}

void destroyMMU(void) {
	ThreadNode* currThread = mmu->tList, *nextThread;
	PageNode *currPage, *nextPage;
	while(currThread != NULL) {
		nextThread = currThread->next;
		currPage = currThread->pageList;
		while(currPage != NULL) {
			nextPage = currPage->next;
			free(currPage);
			currPage = nextPage;
		}
		free(currThread);
		currThread = nextThread;
	}

	free(mmu->active);
	free(mmu->swappedOut);
	free(mmu->threadMap);
	free(mmu->threadMapDisk);
	free(mmu);
	mmu = NULL;
	
}


