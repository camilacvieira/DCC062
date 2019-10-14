/*
*  main.c - Simulacao de carga de trabalho para escalonamento de processos
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico I - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "process.h"
#include "sched.h"
#include "rrprio.h"

#define SCHED_ITERATIONS 10
#define PROCESS_CREATION_PROBABILITY 0.3
#define PROCESS_DESTROY_PROBABILITY 0.2
#define PROCESS_BLOCK_PROBABILITY 0.5
#define PROCESS_UNBLOCK_PROBABILITY 0.4
#define PROCESS_FEATURE_PROBABILITY 0.1

int slotRRP = -1;

Process* _createProcess(Process *plist, int ppid, int slot, void *sp, int priority); 

void _dumpSchedParams(Process *p) {
	if (processGetSchedSlot(p) == slotRRP) {
		RRPSchedParams *rrpsp = processGetSchedParams(p);
		printf("Prio: %d; ",rrpsp->priority);
	}
}

void _doSchedFeature(Process *p) {
	if (processGetSchedSlot(p) == slotRRP) {
            SchedInfo *si = schedGetSchedInfo(slotRRP);
            if (si) {
                RRPSchedParams *rrpsp = malloc(sizeof(RRPSchedParams));
                rrpsp->priority = rand() % RRPRIO_NUMPRIOS;
                si->setSomeFeatureFn(p, rrpsp);
                printf("Modificada Prioridade de PID %d para %d\n",processGetPid(p),rrpsp->priority);
                free(rrpsp);
            }
	}
}

Process* _createRandomProcess(Process *plist, int ppid) {
	if (slotRRP >= 0) {
		int slot = slotRRP;
		RRPSchedParams *rrpsp = malloc(sizeof(RRPSchedParams));
                int priority = (!plist ? 0 : rand() % RRPRIO_NUMPRIOS);
		rrpsp->priority = priority;
		plist = _createProcess(plist, ppid, slotRRP, rrpsp, priority);
		return plist;
	}
	return NULL;	
}


Process* _createProcess(Process *plist, int ppid, int slot, void *sp, int priority) {
	if (!schedGetSchedInfo(slot)) return NULL;
	printf("Criando processo... ");
	plist = processCreate(plist, priority);
	processSetStatus(plist, PROC_READY);
	processSetParentPid(plist, ppid);
	schedSetScheduler(plist, sp, slot);
	printf(" Criado PID %d!\n", processGetPid(plist));
	return plist;
}

Process* _destroyProcess(Process *plist, int pid) {
	printf("Destruindo processo... ");
	plist = processDestroy(plist, pid);
	printf(" Destruido PID %d!\n", pid);
	return plist;
}

Process* _doRandomThings(Process *plist) {
	Process *p, *next, *dst;
	int pid, n;
	int ready;
	double r = rand() / (double)RAND_MAX;
	printf("===Acoes Aleatorias===\n");
	if (r < PROCESS_CREATION_PROBABILITY){
		plist = _createRandomProcess(plist,1);
	}
//        printf("1\n");
	for (p=plist;p!=NULL;p=next) {
		SchedInfo *si = schedGetSchedInfo(processGetSchedSlot(p));
		if (!si) continue;
//                printf("2\n");
		next = processGetNext(p);
		pid = processGetPid(p);
		if (pid==1) continue;
		r = rand() / (double)RAND_MAX;
//                printf("3\n");
		if (processGetStatus(p)==PROC_RUNNING &&
		    r < PROCESS_DESTROY_PROBABILITY) {
			plist = _destroyProcess(plist,processGetPid(p));
			continue;
		}
//                printf("4\n");
		r = rand() / (double)RAND_MAX;
		if (processGetStatus(p)==PROC_RUNNING && 
		    r < PROCESS_BLOCK_PROBABILITY) {
//                    printf("5\n");
			processSetStatus(p,PROC_WAITING);
			si->notifyProcessStatusFn(p,PROC_RUNNING);
			printf("Bloqueado processo %d\n",pid);
		}
		else if (processGetStatus(p)==PROC_WAITING &&
		    r < PROCESS_UNBLOCK_PROBABILITY) {
//                    printf("6\n");
			processSetStatus(p,PROC_READY);
			si->notifyProcessStatusFn(p,PROC_WAITING);
			printf("Desbloqueado processo %d\n",pid);
		}
		else if (processGetStatus(p)==PROC_READY &&
		    r < PROCESS_FEATURE_PROBABILITY) {
                    printf("8\n");
			_doSchedFeature(p);
                    printf("9\n");
		}
//                printf("7\n");
	}
	printf("======================\n");
	return plist;
}

int main(void) {
	int i = 0, step = 0;
	char c = ' ';
	Process* plist = NULL, *p1 = NULL;
	
	srand(time(NULL));
	
	//Inicializar escalonadores de processos
	schedInitSchedInfo();
	slotRRP = rrpInitSchedInfo();
	
	//Criando primeiro processo...	
	plist = _createRandomProcess(plist, 1);
	printf("\n");
	
	while (c != 'n') {
		switch (i) {
			case 0:
				printf("(Passo:%d)\n", step);
				plist = _doRandomThings(plist);
				processDump(plist,_dumpSchedParams);
				printf("\n");
				i++;
				break;
			case SCHED_ITERATIONS+1:
				printf("(Passo:%d/Iteracoes:%d)\n", step, i-1);
				processDump(plist,_dumpSchedParams);
				step++;
				i = 0;
				printf("\nContinuar (s/n)? ");
				fflush(stdout);
				c = getchar();
				printf("\n");
				break;
			default:
				p1 = schedSchedule(plist);
				i++;
		}		
	}
	return 0;
}
