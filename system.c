#include "system.h"
#include <stdlib.h>
#include <stdio.h>

static void __stdcall fiber_entry(void *param)
{
    PTR_DESC d = (PTR_DESC)param;
    if (!d || !d->proc) {
        fprintf(stderr, "fiber_entry: descritor/proc invalido.\n");
        ExitProcess(1);
    }

    d->proc(d->arg);

    /* Se retornar, é erro de uso: no núcleo, processos devem terminar via termina_processo(). */
    fprintf(stderr, "Erro: co-rotina retornou. Use termina_processo() ao final do processo.\n");
    ExitProcess(1);
}

void system_init_main(PTR_DESC d_main)
{
    if (!d_main) {
        fprintf(stderr, "system_init_main: d_main nulo.\n");
        exit(1);
    }

    if (d_main->fiber != NULL) {
        /* já inicializado */
        return;
    }

    /* Converte a thread atual (main) em fiber */
    d_main->fiber = ConvertThreadToFiber(NULL);
    if (d_main->fiber == NULL) {
        fprintf(stderr, "ConvertThreadToFiber falhou. GetLastError=%lu\n", (unsigned long)GetLastError());
        exit(1);
    }

    d_main->proc = NULL;
    d_main->arg  = NULL;
}

PTR_DESC cria_desc(void)
{
    PTR_DESC d = (PTR_DESC)calloc(1, sizeof(descritor));
    if (!d) {
        perror("cria_desc");
        exit(1);
    }
    return d;
}

void newprocess(proc_fn proc, void *arg, PTR_DESC d)
{
    if (!d || !proc) {
        fprintf(stderr, "newprocess: argumentos invalidos.\n");
        exit(1);
    }

    d->proc = proc;
    d->arg  = arg;

    /* CreateFiber(stackSize=0 usa default). Param passa o próprio descritor. */
    d->fiber = CreateFiber(0, fiber_entry, d);
    if (d->fiber == NULL) {
        fprintf(stderr, "CreateFiber falhou. GetLastError=%lu\n", (unsigned long)GetLastError());
        exit(1);
    }
}

void transfer(PTR_DESC origem, PTR_DESC destino)
{
    (void)origem; /* em fibers, o estado da fiber atual é preservado automaticamente */

    if (!destino || !destino->fiber) {
        fprintf(stderr, "transfer: destino invalido.\n");
        exit(1);
    }

    SwitchToFiber(destino->fiber);
}
