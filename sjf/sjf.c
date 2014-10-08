#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../utils.h"

List* processList;

void parseInput(void) {
	char dir[1024], nome[64];
	int tempo, pid;

	while(scanf(" exec %s tempoexec=%d", nome, &tempo)==2) {
		getcwd(dir,1024);
		strcat(dir,"/../");
		strcat(dir,nome);
		pid = fork();
		if(pid==0) {
			execvp(dir,NULL);
		} else {
			kill(pid, SIGSTOP);
			storeProcess(processList, pid, tempo);
			printf("%s %d %d\n", nome, tempo, pid);
		}
	}

	reorderList(processList);

}

int main (void) {
	Process *current;
	redirectIO();
	processList = createList();
	parseInput();

	current = processList->first;
	while(current != NULL) {
		printf("chamando %d\n", current->pid);
		kill(current->pid, SIGCONT);
		waitpid(current->pid, NULL, 0);
		printf("concluido\n");
		current = current->next;
	}

	destroyList(processList);

	return 0;
}