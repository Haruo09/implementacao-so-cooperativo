#ifndef SYSTEM_H
#define SYSTEM_H

/* Implementação de co-rotinas para GNU GCC no Windows usando Fibers */
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #error "Esta versão (fibers) foi preparada para Windows (_WIN32)."
#endif

typedef void (*proc_fn)(void *arg);

/* Descritor de contexto (co-rotina) */
typedef struct {
    LPVOID  fiber;   /* handle da fiber */
    proc_fn proc;    /* função de entrada */
    void   *arg;     /* argumento para a função */
} descritor;

typedef descritor* PTR_DESC;

/* Inicializa o contexto do main como fiber (obrigatório antes de transferir) */
void system_init_main(PTR_DESC d_main);

/* Cria descritor dinamicamente */
PTR_DESC cria_desc(void);

/* Cria uma co-rotina (fiber) que iniciará em proc(arg) */
void newprocess(proc_fn proc, void *arg, PTR_DESC d);

/* Transfere controle entre co-rotinas */
void transfer(PTR_DESC origem, PTR_DESC destino);

#endif
