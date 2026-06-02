#ifndef NUCLEO_H_INCLUDED
#define NUCLEO_H_INCLUDED

#include "system.h"

typedef enum
{
    ATIVO,
    BLOQ_P,
    TERMINADO
} ESTADO_PROC;

typedef struct desc_p
{
    char          nome[35];
    ESTADO_PROC   estado;

    // Contexto (fiber)
    PTR_DESC      contexto;

    // Ponteiros de filas
    struct desc_p *fila_sem;
    struct desc_p *prox_desc;

    // Função do processo (para o trampolim)
    void (*codigo)(void);
} DESCRITOR_PROC;

typedef DESCRITOR_PROC* PTR_DESC_PROC;

void cria_processo(void (*end_proc)(void), const char *nome_p);
void inicia_fila_prontos(void);
void dispara_sistema(void);
void yield(void);
void termina_processo(void);
void encerra_fila_prontos(void);

#endif // NUCLEO_H_INCLUDED
