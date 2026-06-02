#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "system.h"

#include "nucleo.h"

PTR_DESC_PROC prim   = NULL;
PTR_DESC_PROC ultimo = NULL;
PTR_DESC_PROC atual  = NULL;

static descritor main_desc;
static PTR_DESC  main_ctx   = &main_desc;
static int       main_ready = 0;

// ===========================  FUNÇÕES DE DEBUG DA BIBLIOTECA  ===========================
static void print_descritor(PTR_DESC_PROC d)
{
    printf("[DEBUG] DESCRITOR: %s. STATUS: ", d->nome, d->estado);
    switch (d->estado) {
    case ATIVO:
        printf("ATIVO\n");
        break;
    case BLOQ_P:
        printf("BLOQ_P\n");
        break;
    case TERMINADO:
        printf("TERMINADO\n");
        break;
    default:
        printf("Estado Inválido\n");
    }
}

static void print_fila_circular(void)
{
    PTR_DESC_PROC aux = prim->prox_desc;
    print_descritor(prim);
    while (aux && aux != prim)
    {
        print_descritor(aux);
        aux = aux->prox_desc;
    }
}

// =========================== FUNÇÕES AUXILIARES DA BIBLIOTECA ===========================

// TODO: static inserir_processo_na_fila(...); [x]
// TODO: static liberar_processo_na_fila(...); [c] task cancelada.
// TODO: static set_main_ready(...);           [x]
// TODO: static processo_trampolin(...);       [x]
// TODO: static proximo_ativo_depois(...);     [x]

static void inserir_processo_na_fila(PTR_DESC_PROC proc)
{
    if (!prim && !ultimo) {
        prim = proc;
        ultimo = proc;
        proc->prox_desc = proc;
    }

    else {
        ultimo->prox_desc = proc;
        ultimo = proc;
        ultimo->prox_desc = prim;
    }
}

// A implementação a seguir é válida, mas acabou não utilizada.
static void liberar_processo_na_fila(PTR_DESC_PROC proc)
{
    if (!proc || proc->estado != TERMINADO) {
        fprintf(stderr, "[ERRO] %s, linha %d. Descritor de processo inválido ou não terminado.\n", __FUNCTION__, __LINE__);
        return;
    }

    if (proc->contexto) {
        if (proc->contexto->fiber) {
            DeleteFiber(proc->contexto->fiber);
        }
        free(proc->contexto);
        proc->contexto = NULL;
    }

    printf("[DEBUG] Processo %s liberto da memória de contexto.\n", proc->nome);
}

static void set_main_ready()
{
    PTR_DESC_PROC p = prim;
    while (p != ultimo) {
        if (p->estado != TERMINADO) {
            main_ready = 0;
            return;
        }
        p = p->prox_desc;
    }

    main_ready = ultimo->estado == TERMINADO;  // atribui 1 se verdade, 0 se falso.
}


static void processo_trampolim(void *arg)
{
    PTR_DESC_PROC ptr_desc_proc = (PTR_DESC_PROC) arg;
    if (!ptr_desc_proc || !ptr_desc_proc->codigo) {
        fprintf(stderr, "[ERRO]: %s, linha %d. Descritor ou código inválido.\n", __FUNCTION__, __LINE__);
        return;
    }

    atual = ptr_desc_proc;
    ptr_desc_proc->codigo();

    termina_processo();
    yield();

    // Caso o yield() não tenha transferido, é porque não há nenhum outro processo 'PRONTO'.
    // Sendo assim, verifica se pode transferir pra main().
    if (main_ready) {
        printf("[DEBUG] %s: pronto pra transferir pra main().\n", __FUNCTION__);
        transfer(atual->contexto, main_ctx);
    }

    else {
        fprintf(stderr, "[ERRO]: %s, linha %d. Não foi possível retornar para a fiber principal.\n", __FUNCTION__, __LINE__);
        exit(1);
    }
}

static PTR_DESC_PROC proximo_ativo_depois(PTR_DESC_PROC a_partir)
{
    PTR_DESC_PROC aux = a_partir->prox_desc;

    while (aux != a_partir) {
        if (aux->estado == ATIVO)
            return aux;
        aux = aux->prox_desc;
    }

    return (a_partir->estado == ATIVO) ? a_partir : NULL;
}

// =========================== FUNÇÕES PRINCIPAIS DA BIBLIOTECA ===========================

// TODO: cria_processo(...);        [x]
// TODO: inicia_fila_prontos(...);  [x]
// TODO: dispara_sistema(...);      [x]
// TODO: yield(...);                [x]
// TODO: termina_processo(...);     [x]

void cria_processo(void (*end_proc)(void), const char *nome_p)
{
    PTR_DESC_PROC descritor_processo = (PTR_DESC_PROC) malloc(sizeof(DESCRITOR_PROC));
    if (!descritor_processo) {
        fprintf(stderr, "[ERRO]: %s, linha %d. Não foi possível criar descritor do processo %s.\n", __FUNCTION__, __LINE__, nome_p);
        return;
    }

    strcpy(descritor_processo->nome, nome_p);
    descritor_processo->estado = ATIVO;
    descritor_processo->codigo = end_proc;
    descritor_processo->fila_sem = NULL;

    descritor_processo->contexto = cria_desc();
    newprocess(processo_trampolim, descritor_processo, descritor_processo->contexto);

    inserir_processo_na_fila(descritor_processo);
}

void inicia_fila_prontos(void)
{
    prim   = NULL;
    ultimo = NULL;
    atual  = NULL;
}

void encerra_fila_prontos(void)
{
    PTR_DESC_PROC p = prim;
    PTR_DESC_PROC aux;

    if (ultimo) {
        ultimo->prox_desc = NULL;  // Transforma lista circular em lista linear comum
    }

    while (p != NULL) {
        // não verifica se P é válido ou se já encerrou. Isolamento de responsabilidades.
        DeleteFiber(p->contexto->fiber);
        aux = p->prox_desc;

        printf("[DEBUG] %s: Libertando Fiber %s.\n", __FUNCTION__, p->nome);
        free(p->contexto);
        free(p);
        p = aux;
    }

    prim   = NULL;
    ultimo = NULL;
    atual  = NULL;

    printf("[DEBUG] Todas as Fibers e descritores foram libertos com sucesso pelo processo principal.\n");
}

void dispara_sistema(void)
{
    if (prim == NULL) return;

    system_init_main(main_ctx);

    atual = (prim->estado == ATIVO) ? prim : proximo_ativo_depois(prim);

    printf("[DEBUG] Processos a serem executados:\n");
    print_fila_circular();
    if (atual != NULL) {
        transfer(main_ctx, atual->contexto);
    }
}

void yield(void)
{
    PTR_DESC_PROC prox;

    if (atual == NULL) return;

    prox = proximo_ativo_depois(atual);

    if (prox != NULL && prox != atual) {
        PTR_DESC_PROC antigo = atual;
        atual = prox;

        transfer(antigo->contexto, atual->contexto);

    }
    else {
        set_main_ready();  // Atribui a variável `main_ready` como 1 se não houver outros ATIVOS.
    }
}

void termina_processo(void)
{
    if (!atual || atual->estado != ATIVO) {
        fprintf(stderr, "[ERRO]: %s, linha %d. Não foi possível terminar o processo.\n", __FUNCTION__, __LINE__);
        return;
    }

    atual->estado = TERMINADO;

    // PTR_DESC_PROC antigo = atual;
    // liberar_processo_na_fila(antigo);
}
