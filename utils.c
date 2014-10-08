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

void storeProcess( List* list, int pid, int scale ) {
	Process *p, *new;
	new = (Process *)malloc(sizeof(Process));
	new->pid = pid;
	new->scale = scale;

	p = list->first;
	list->first = new;
	new->next = p;

	list->count++;
}

int compare (const void * a, const void * b) {
	return (*(Process**)a)->scale - (*(Process**)b)->scale;
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

