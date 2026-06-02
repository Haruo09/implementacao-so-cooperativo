#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "nucleo.h"
#include "semaforo.h"
#include "pausa.h"

#define TAM 7
#define QTD_VALUES 12

int buffer[TAM] = {0};
int indiceEntrada = 0;
int indiceSaida = 0;

semaforo cheio;
semaforo vazio;
semaforo exclusaoMutua;

void depositar(int mensagemArg) {
    buffer[indiceEntrada] = mensagemArg;
    printf("[Produtor] Depositou %d na posicao %d.\n", mensagemArg, indiceEntrada);
    indiceEntrada = (indiceEntrada + 1) % TAM;
}

int retirar(void) {
    int mensagemLida = buffer[indiceSaida];
    buffer[indiceSaida] = 0;
    printf("[Consumidor] Retirou %d da posicao %d.\n", mensagemLida, indiceSaida);
    indiceSaida = (indiceSaida + 1) % TAM;
    return mensagemLida;
}

void rotinaProdutor(void) {
    int mensagemAtual = 1;

    for (mensagemAtual = 1; mensagemAtual <= QTD_VALUES; mensagemAtual++){

        so_delay_ms(200);

        P(&vazio);
        P(&exclusaoMutua);

        depositar(mensagemAtual);

        V(&exclusaoMutua);
        V(&cheio);

    }
    printf("[DEBUG] %s encerrado.\n", __FUNCTION__);
    yield();
}

void rotinaConsumidor(void) {
    int limite;
    for (limite = 1; limite <= QTD_VALUES; limite++) {
        P(&cheio);
        P(&exclusaoMutua);

        retirar();

        V(&exclusaoMutua);
        V(&vazio);

        so_delay_ms(500);
    }

    printf("[DEBUG] %s encerrado.\n", __FUNCTION__);
    yield();
}

int main(void) {
    printf("=== Produtor/Consumidor ===\n");

    indiceEntrada = 0;
    indiceSaida = 0;

    inicia_fila_prontos();

    inicia_semaforo(&cheio, 0);
    inicia_semaforo(&vazio, TAM);
    inicia_semaforo(&exclusaoMutua, 1);

    cria_processo(rotinaProdutor, "Produtor");
    cria_processo(rotinaConsumidor, "Consumidor");

    printf("[MAIN] Iniciando escalonamento...\n\n");
    dispara_sistema();

    encerra_fila_prontos();
    printf("\n\n[MAIN] Escalonamento finalizado.\n");

    return 0;
}
