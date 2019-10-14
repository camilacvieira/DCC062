/*
*  sched.c - Implementacao da API do escalonador de processos
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
#include <string.h>
#include "sched.h"

#define MAX_NUM_SLOT 4

SchedInfo* sched_slots[MAX_NUM_SLOT];

//Inicializa as informacoes sobre escalonadores
void schedInitSchedInfo(void) {
	int i;
	//Inicializar slots de registro de escaloandores
	for (i=0; i<MAX_NUM_SLOT; i++)
		sched_slots[i]=NULL;
}

//Retorna informacoes sobre escalonador de um dado slot
SchedInfo* schedGetSchedInfo(int slot) {
	if (slot>=0 && slot<=MAX_NUM_SLOT)
		return sched_slots[slot];
	else return NULL;
}

//Aciona o escalonador de processos, que decide qual algoritmo deve ser usado 
//e delega a esse algoritmo a decisao sobre qual processo obtera' a CPU
//Retorna NULL caso nao haja um processo pronto para assumir a CPU (idle)
//ou ponteiro para o processo escolhido
Process* schedSchedule(Process *plist) {

	Process *newp, *oldp;

	//Como por enquanto teremos apenas um algortimo, o trabalho de schedule
	//e' bem simples. Vamos simplesmente delegar a decisao de escalonamento
	//ao algoritmo registrado no primeiro slot

	if (sched_slots[0]==NULL) return NULL;

	//Se houver algum processo em execucao, colocar como pronto
	if ((oldp=processGetByStatus(plist,PROC_RUNNING))) { //Atribuicao mesmo!
		processSetStatus(oldp,PROC_READY);
		sched_slots[0]->notifyProcessStatusFn(oldp,PROC_RUNNING);
	}

	newp = sched_slots[0]->scheduleFn(plist);

	//Colocar processo escolhido como RUNNING
	if (newp) {
		int oldstatus = processGetStatus(newp); //Deve ser READY!
		processSetStatus(newp,PROC_RUNNING);
		sched_slots[0]->notifyProcessStatusFn(newp,oldstatus);
		processAddCpuUsage(newp,1);
	}

	return newp;
}

//Associa um processo a um algoritmo de escalonamento especifico
//Retorna negativo caso o algoritmo de escalonamento nao seja encontrado
int schedSetScheduler(Process *p, void *params, int slot) {
	int oldslot;
	//Verificar se slot e' valido
	if (sched_slots[slot]==NULL) return -1;

	oldslot = processGetSchedSlot(p);
	//Liberar parametros de escalonamento antigos
	if (oldslot>=0)
		sched_slots[oldslot]->releaseParamsFn(p);

	//Inicializar dos parametros de escalonamento para o processo
	sched_slots[slot]->initParamsFn(p, params);

	return 1;
}

//Registra um algoritmo de escalonamento. O endereco apontado por si sera'
//usado diretamente! Se bem sucedido, retorna o numero do slot ocupado ou
//negativo caso contrario
int schedRegisterScheduler(SchedInfo *si) {
	int i;
	//Tentar encontrar slot livre
	for (i=0; i<MAX_NUM_SLOT && sched_slots[i]!=NULL; i++);
	if (i==MAX_NUM_SLOT) return -1;

	//Atribuir ao slot a estrutura com informacoes do novo escalonador	
	sched_slots[i] = si;

	return i;
}

//Remove do escalonador um algoritmo de escalonamento
//Retorna numero do slot liberado se bem sucedido ou um negativo caso contrario
int schedUnregisterScheduler(int slot, char *name) {
	//Verificar se slot informado e' valido
	if (!sched_slots[slot] || strcmp(name,sched_slots[slot]->name))
		return -1;
	//Forcar valor de slot para NULL
	sched_slots[slot]=NULL;
	return slot;
}
