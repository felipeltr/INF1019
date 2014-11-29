#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "../utils.h"

#define QUANTUM 1

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
			execl(dir,nome,NULL);
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

void activateHandler(void) {
	if(signal(SIGALRM, sigHandler) == SIG_ERR || signal(SIGCHLD, sigHandler) == SIG_ERR) {
		perror("Erro signal handler");
		exit(1);
	}
}

void deactivateHandler(void) {
	if(signal(SIGALRM, SIG_IGN) == SIG_ERR || signal(SIGCHLD,  SIG_IGN) == SIG_ERR) {
		perror("Erro signal handler");
		exit(1);
	}
}

int main (void) {
	redirectIO();
	processList = createList();
	parseInput();

	current = processList->first;
	while(current != NULL) {
		printf("[ESCALONADOR] chamando processo %d...\n",current->pid);
		fflush(stdout);
		kill(current->pid, SIGCONT);
		activateHandler();
		alarm( QUANTUM );
		printf("[ESCALONADOR] entrando em espera...\n");
		fflush(stdout);
		pause();
		printf("[ESCALONADOR] saindo da espera...\n");
		fflush(stdout);
		deactivateHandler();
		current = finished ? removeProcessAndGetNext(processList, current) : current->next;
		if(!current) current = processList->first;

		finished = 0;
	}

	printf("[ESCALONADOR] Todos os processos terminaram!\n");
	fflush(stdout);

	destroyList(processList);

	return 0;
}