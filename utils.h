typedef struct tprocess {
	int pid;
	int scale; // prioridade ou tempo de execucao, dependendo do caso
	int scale2; // instante de tempo que o processo entra na lista, se aplicavel
	struct tprocess *next;
} Process;

typedef struct tlist {
	Process *first;
	int count;
} List;

List* createList(void) ;
void storeProcess( List* list, int pid, int scale ) ;
void storeProcess2( List* list, int pid, int scale, int scale2 ) ;
void reorderList( List* list ) ;
Process* removeProcessAndGetNext(List* list, Process* p) ;
void destroyList( List* list ) ;

void redirectIO(void) ;