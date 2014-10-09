#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../utils.h"

#define QUANTUM 3

List* processList;
Process *current;
int isAlarm = 0, finished = 0;

void parseInput(void) {
	char dir[1024], nome[64];
	int pid;

	while(scanf(" exec %s", nome)==1) {
		getcwd(dir,1024);
		strcat(dir,"/../dummies/");
		strcat(dir,nome);
		pid = fork();
		if(pid==0) {
			execvp(dir,NULL);
		} else {
			kill(pid, SIGSTOP);
			storeProcess(processList, pid, 0);
		}
	}

	reorderList(processList);

}

void sigHandler(int sig) {
	if(sig == SIGALRM) {
		isAlarm = 1;
		printf("processo %d estourou o tempo...\n",current->pid);
		fflush(stdout);
		kill(current->pid,SIGSTOP);
	} else if(sig == SIGCHLD) {
		if(!isAlarm) {	// verifica se a alteracao no filho foi termino de execucao (e nao preempcao)
			alarm(0);
			finished = 1;
			printf("processo %d acabou sua execucao...\n",current->pid);
			fflush(stdout);
		}
		isAlarm = 0;
	}
}

int main (void) {
	redirectIO();
	processList = createList();
	parseInput();

	if(signal(SIGALRM, sigHandler) == SIG_ERR || signal(SIGCHLD, sigHandler) == SIG_ERR) {
		perror("Erro signal handler");
		exit(1);
	}

	current = processList->first;
	while(current != NULL) {
		printf("chamando processo %d...\n",current->pid);
		kill(current->pid, SIGCONT);
		alarm( QUANTUM );
		printf("escalonador entrando em espera...\n");
		fflush(stdout);
		pause();
		printf("escalonador saindo da espera...\n");
		fflush(stdout);
		current = finished ? removeProcessAndGetNext(processList, current) : current->next;
		if(!current) current = processList->first;

		finished = 0;
	}

	printf("Todos os processos terminaram!\n");
	fflush(stdout);

	destroyList(processList);

	return 0;
}