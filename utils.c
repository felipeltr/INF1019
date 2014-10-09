#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"


List* createList(void) {
	List* list;
	list = (List*) malloc(sizeof(List));
	list->first = NULL;
	list->count = 0;

	return list;
}

void storeProcess2( List* list, int pid, int scale, int scale2 ) {
	Process *p, *new;
	new = (Process *)malloc(sizeof(Process));
	new->pid = pid;
	new->scale = scale;
	new->scale2 = scale2;
	new->next = NULL;

	p = list->first;
	if(!p)
		list->first = new;
	else {
		while(p->next != NULL)
			p = p->next;
		p->next = new;		
	}

	list->count++;
}

void storeProcess( List* list, int pid, int scale ) {
	storeProcess2( list, pid, scale, 0);
}

int compare (const void * a, const void * b) {
	int diff;
	diff = (*(Process**)a)->scale - (*(Process**)b)->scale;
	if(!diff)
		diff = (*(Process**)a)->scale2 - (*(Process**)b)->scale2;
	return diff;
}

void reorderList( List* list ) {
	Process **array, *current;
	int i = 0;

	if(list->count <= 1) return;
	array = (Process **)malloc(list->count * sizeof(Process*));

	current = list->first;
	while(current != NULL) {
		array[i++] = current;
		current = current->next;
	}

	qsort(array, list->count, sizeof(Process*), compare);

	list->first = array[0];
	current = array[0];
	for(i=1;i<list->count;i++) {
		current->next = array[i];
		current = current->next;
	}
	current->next = NULL;

}

Process* removeProcessAndGetNext(List* list, Process* p) {
	Process *current, *prev = NULL;
	current = list->first;
	while(current != p) {
		prev = current;
		current = current->next;
	}

	if(prev == NULL)
		list->first = p->next;
	else
		prev->next = p->next;

	current = p->next;

	free(p);
	return current;
}

void destroyList( List* list ) {
	Process *current, *next;

	current = list->first;
	while(current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}

	free(list);
}

void redirectIO(void) {
	int fd;
	if ((fd=open("entrada.txt",O_RDONLY)) == -1){
		perror("Erro entrada.txt");
		exit(1);
	}
	if(dup2(fd, 0) == -1) {
		perror("Erro STDIN redirect");
		exit(1);
	}
}

