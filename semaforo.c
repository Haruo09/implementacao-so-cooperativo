#include "semaforo.h"
#include <stdio.h>
#include <stdlib.h>

extern PTR_DESC_PROC prim;
extern PTR_DESC_PROC atual;
extern void yield(void);

void inicia_semaforo(semaforo *sem, int n) {
    if (!sem) return;
    sem->s = n;
    sem->Q = NULL;
}

void P(semaforo *sem) {
    PTR_DESC_PROC aux;
    PTR_DESC_PROC proximoAtivo;

    if (!sem) return;

    if (!atual) {
        fprintf(stderr, "P: nenhum processo atual...\n");
        exit(1);
    }

    if (sem->s > 0) {
        sem->s--;
        return;
    }

    atual->estado = BLOQ_P;
    atual->fila_sem = NULL;

    if (!sem->Q) {
        sem->Q = atual;
    } else {
        aux = sem->Q;
        while (aux->fila_sem) aux = aux->fila_sem;
        aux->fila_sem = atual;
    }

    proximoAtivo = NULL;
    if (prim) {
        PTR_DESC_PROC currentScan = atual->prox_desc;
        while (currentScan && currentScan != atual) {
            if (currentScan->estado == ATIVO) {
                proximoAtivo = currentScan;
                break;
            }
            currentScan = currentScan->prox_desc;
        }
        if (!proximoAtivo && atual->estado == ATIVO) proximoAtivo = atual;
    }

    if (!proximoAtivo) {
        fprintf(stderr, "Deadlock!\n");
        exit(1);
    }

    yield();
}

void V(semaforo *sem) {
    PTR_DESC_PROC processoLiberado;

    if (!sem) return;

    if (sem->Q == NULL) {
        sem->s++;
    } else {
        processoLiberado = sem->Q;
        sem->Q = processoLiberado->fila_sem;
        processoLiberado->fila_sem = NULL;

        processoLiberado->estado = ATIVO;
    }
}
