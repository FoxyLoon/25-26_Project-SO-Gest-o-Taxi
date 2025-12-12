#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

// Strings e funções de manipulação
#include <string.h>

// open(), close(), read(), write(), access(), unlink(), ...
#include <unistd.h>

// Permissões de Leitura e Escrita (RDWR, RDONLY, WRONLY,...)
#include <fcntl.h>

// Modificação de Caracteres (toupper, tolower,...)
#include <ctype.h>

// mkfifo()
#include <sys/stat.h>
#include <sys/types.h>

// FIle Descriptors (FD_SET, FD_ZERO, FD_ISSET, FD_CLR, FD_OK,...)
#include <sys/select.h>

// Threads
#include <pthread.h>

// Sinais
#include <signal.h>

// =====================================

// Definicoes
#define NVEICULOS 10
#define NCLIENTES 30
#define NSERVICOS 50

// =====================================

// Tamanhos de Strings (Não modificar)
#define TAM 100
#define TAM_CMD 30

// FIFOs
#define SRV_FIFO "fifo_servidor"
#define CLI_FIFO "fifo_cliente_%d"

// =====================================

int km_total = 0;
int hora_simulada = 0;
int servidor_ativo = 1;
int CLIENTES_ATIVOS = 0;
int SERVICOS_ATIVOS = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// =====================================

// Estruturas
typedef struct 
{
    char username[TAM];
    int pid_cliente;
} CLIENTE;


typedef struct {
    int pid_cliente;
    char username[TAM];
    char comando[TAM_CMD];
    int hora;
    int id_servico;
    char origem[TAM];
    int distancia;
} PEDIDO_CLIENTE;

typedef struct {
    char msg[TAM];
} RESPOSTA_SERVER;

typedef struct {
    int id_servico;
    int status_servico;
    int pid_cliente;
    int pid_veiculo;
    char username[TAM];
    int hora;
    char origem[TAM];
    int distancia;
} SERVICO;

// =====================================

// Quantidades de Clientes e Servicos
CLIENTE clientes[NCLIENTES];
SERVICO servicos[NSERVICOS];

// =====================================
// Funções Globais

// Verifica se o char s é um número
int isNumber(const char *s) {
    for (int i = 0; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

#endif