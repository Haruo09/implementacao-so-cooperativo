#ifndef SEMAFORO_H
#define SEMAFORO_H

#include "nucleo.h"

typedef struct {
    int s;
    PTR_DESC_PROC Q;
} semaforo;

void inicia_semaforo(semaforo *sem, int n);
void P(semaforo *sem);
void V(semaforo *sem);

#endif
