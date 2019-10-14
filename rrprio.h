/*
*  rrprio.h - Header da API do algoritmo Round Robin com Prioridades 
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico I - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => TENTE MODIFICAR APENAS A STRUCT rrp_params <=
*
*/

#ifndef RRPRIO_H
#define RRPRIO_H

#include "sched.h"

#define RRPRIO_NUMPRIOS 5

typedef struct rrp_params {
        int priority;
        //...
        //...
} RRPSchedParams;

//Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
//conforme o algoritmo Round Robin com Prioridades
//Deve envolver a inicializacao de possiveis parametros gerais
//Deve envolver o registro do algoritmo junto ao escalonador
//Retorna o numero do slot obtido no registro do algoritmo junto ao escalonador
int rrpInitSchedInfo();

#endif
