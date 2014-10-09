#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "../utils.h"

#define QUANTUM 3

List* processList;
Process *current;
int isAlarm = 0, finished = 0;

void parseInput(void) {
	char dir[1024], nome[64];
	int pid, priority;

	while(scanf(" exec %s prioridade=%d", nome, &priority)==2) {
		getcwd(dir,1024);
		strcat(dir,"/../dummies/");
		strcat(dir,nome);
		pid = fork();
		if(pid==0) {
			execvp(dir,NULL);
		} else {
			kill(pid, SIGSTOP);
			storeProcess2(processList, pid, priority, clock());
		}
	}

	reorderList(processList);

}

void sigHandler(int sig) {
	if(sig == SIGALRM) {
		isAlarm = 1;
		printf("[ESCALONADOR] processo %d estourou o tempo...\n",current->pid);
		fflush(stdout);
		kill(current->pid,SIGSTOP);
	} else if(sig == SIGCHLD) {
		if(!isAlarm) {	// verifica se a alteracao no filho foi termino de execucao (e nao preempcao)
			alarm(0);
			finished = 1;
			printf("[ESCALONADOR] processo %d acabou sua execucao...\n",current->pid);
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
		printf("[ESCALONADOR] chamando processo %d (prioridade %d)...\n",current->pid, current->scale);
		kill(current->pid, SIGCONT);
		alarm( QUANTUM );
		printf("[ESCALONADOR] entrando em espera...\n");
		fflush(stdout);
		pause();
		printf("[ESCALONADOR] saindo da espera...\n");
		fflush(stdout);
		if(current->scale != 7) current->scale++;
		current->scale2 = clock();
		if(finished) removeProcess(processList, current);
		reorderList(processList);
		current = processList->first;

		finished = 0;
	}

	printf("[ESCALONADOR] Todos os processos terminaram!\n");
	fflush(stdout);

	destroyList(processList);

	return 0;
}