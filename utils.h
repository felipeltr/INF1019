typedef struct tprocess {
	int pid;
	int scale; // prioridade ou tempo de execucao, dependendo do caso
	struct tprocess *next;
} Process;

typedef struct tlist {
	Process *first;
	int count;
} List;

List* createList(void) ;
void storeProcess( List* list, int pid, int scale ) ;
void reorderList( List* list ) ;
void destroyList( List* list ) ;

void redirectIO(void) ;