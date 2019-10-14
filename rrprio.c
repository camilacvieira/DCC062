/*
*  rrprio.c - Implementacao do algoritmo Round Robin com Prioridades e sua API
*
*  Autores: Caio Vincenzo Reis Dima, Camila Corrêa Vieira, Pedro Cotta Badaró
*  Projeto: Trabalho Pratico I - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*
*/

#include "rrprio.h"
#include <stdio.h>
#include <string.h>

//Nome unico do algoritmo. Deve ter 6 caracteres no máximo.
const char rrpName[]="RRPrio";
//=====Funcoes Auxiliares=====
//...
int schedRRPRIOslot = -1;

typedef struct process_node{
	Process *process;
	struct process_node *next;
} ProcessNode;

typedef struct queue_process_by_priority{
	ProcessNode *first;
	ProcessNode *last;
} QueueProcessByPriority;

QueueProcessByPriority priorityQueues[RRPRIO_NUMPRIOS];

void enqueue(QueueProcessByPriority *queue, Process *process){
	ProcessNode *processNode = malloc(sizeof(processNode));
	processNode->process = process;
	processNode->next = NULL;
	if(queue->first == NULL){
            queue->first = processNode;
            queue->last = processNode;
	}
	else{
            queue->last->next = processNode;
            queue->last = processNode;
	}
}

void removeProcessFromQueue(QueueProcessByPriority *queue, Process *process){
	ProcessNode *currentProcessNode = queue->first;
	ProcessNode *prevProcessNode = NULL;
	while(currentProcessNode != NULL && processGetPid(currentProcessNode->process) != processGetPid(process)){
		prevProcessNode = currentProcessNode;
		currentProcessNode = currentProcessNode->next;
	}
        if(currentProcessNode != NULL){
            if(prevProcessNode != NULL){
                prevProcessNode->next = currentProcessNode->next;
            }
            else{
                if(currentProcessNode->next == NULL)
                    queue->last = NULL;
                queue->first = currentProcessNode->next;
            }
            currentProcessNode->next = NULL;
            currentProcessNode->process = NULL;
            free(currentProcessNode);
        }
}

ProcessNode* dequeue(QueueProcessByPriority *queue){
	ProcessNode *processNode = queue->first;
	if(processNode != NULL){
            queue->first = processNode->next;
            if(queue->first == NULL)
                queue->last = NULL;
	}
	return processNode;
}

Process *requeue(QueueProcessByPriority *queue){
	ProcessNode *processNodeFromQueue = dequeue(queue);
	if(processNodeFromQueue != NULL){
		Process *processFromQueue = processNodeFromQueue->process;
		enqueue(queue, processFromQueue);
		return processFromQueue;
	}
	return NULL;
}

void clearQueue(QueueProcessByPriority *queue){
	if(queue != NULL){
		ProcessNode *processNode = queue->first;
		while(processNode != NULL){
			ProcessNode *aux = processNode->next;
			processNode->next = NULL;
			free(processNode);
			processNode = aux;
		}
		queue->first = NULL;
		queue->last = NULL;
		free(queue);
	}
	queue = malloc(sizeof(QueueProcessByPriority));
}


//=====Funcoes da API=====

//Inicializa os parametros de escalonamento de um processo p. Funcao chamada 
//normalmente quando o processo deve ser associado ao algoritmo RRPrio
void rrpInitSchedParams(Process *p, void *rrparams) {
    RRPSchedParams *rrpSchedParams = (RRPSchedParams*)rrparams;
    processSetSchedParams(p, rrpSchedParams);
    processSetSchedSlot(p, schedRRPRIOslot);
    enqueue(&priorityQueues[((RRPSchedParams*)rrpSchedParams)->priority], p);
}

//Retorna o proximo processo a obter a CPU, conforme o algortimo RRPrio 
Process* rrpSchedule(Process *plist) {
	if(!plist) return NULL;
	int priorityIterator = 4;
	while(priorityIterator >= 0) {
		if((priorityQueues[priorityIterator]).first != NULL){
			Process *process = requeue(&priorityQueues[priorityIterator]);
			while(process != NULL && processGetStatus(process) != PROC_READY)
				process = requeue(&priorityQueues[priorityIterator]);
			if(process != NULL)
				return process;
		}
		priorityIterator--;
	}
	return NULL;
}

//Libera os parametros de escalonamento de um processo p. Funcao chamada 
//normalmente quando o processo e' desassociado do slot de RRPrio
//Retorna o numero do slot ao qual o processo estava associado
int rrpReleaseParams(Process *p) {
	RRPSchedParams *params = ((RRPSchedParams*)processGetSchedParams(p));
	removeProcessFromQueue(&priorityQueues[params->priority], p);
	//free(params);
	return schedRRPRIOslot;
}

//Modifica a prioridade atual de um processo
//E' a funcao de setSomeFeatureFn() de RRPrio
void rrpSetPrio(Process *p, void *rrparams) {
    RRPSchedParams *rrpSchedParams = (RRPSchedParams*)processGetSchedParams(p);
    int oldPrio = rrpSchedParams->priority;
    RRPSchedParams *rrpSched = (RRPSchedParams*)rrparams;
    if(oldPrio != rrpSched->priority){
        rrpReleaseParams(p);
        rrpSchedParams = (RRPSchedParams*)malloc(sizeof(RRPSchedParams));
        rrpSchedParams->priority = rrpSched->priority;
        rrpInitSchedParams(p, rrpSchedParams);
    }
}

//Notifica a mudanca de status de um processo para possivel manutencao de dados
//internos ao algoritmo RRPrio, responsavel pelo processo
void rrpNotifyProcessStatus(Process *p, int oldstatus) {
        
    int priority = ((RRPSchedParams*)processGetSchedParams(p))->priority;
    if(processGetStatus(p) == PROC_WAITING && oldstatus == PROC_RUNNING){							// será bloquedo
        removeProcessFromQueue(&priorityQueues[priority], p);
    }
    else if(processGetStatus(p) == PROC_READY && oldstatus == PROC_WAITING){																		// Estava bloqueado, agora estará pronto
        enqueue(&priorityQueues[priority], p);
    }
}


//Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
//conforme o algoritmo Round Robin com Prioridades
//Deve envolver a inicializacao de possiveis parametros gerais
//Deve envolver o registro do algoritmo junto ao escalonador
//Retorna o numero do slot obtido no registro do algoritmo junto ao escalonador
int rrpInitSchedInfo() {
	SchedInfo *sched_info = (SchedInfo*)malloc(sizeof(SchedInfo));
	strcpy(sched_info->name, rrpName);
	sched_info->notifyProcessStatusFn = rrpNotifyProcessStatus;
	sched_info->releaseParamsFn = rrpReleaseParams;
	sched_info->scheduleFn = rrpSchedule;
	sched_info->setSomeFeatureFn = rrpSetPrio;
	sched_info->initParamsFn = rrpInitSchedParams;
	schedRRPRIOslot =  schedRegisterScheduler(sched_info);
	return schedRRPRIOslot;
}
