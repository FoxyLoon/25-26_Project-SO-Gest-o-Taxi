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

// Estruturas
typedef struct
{
    char username[TAM];
    int pid_cliente;
} CLIENTE;

typedef struct
{
    int pid_cliente;
    char username[TAM];
    char comando[TAM_CMD];
    int hora; // Das 0h às 23h
    int id_servico;
    char origem[TAM];
    int distancia;
} PEDIDO_CLIENTE;

typedef struct
{
    char msg[TAM];
} RESPOSTA_SERVER;

typedef struct
{
    int id_servico;
    char username[TAM];
    int pid_cliente;
    int status_servico; // 0 = Sem Agendamento | 1 = Agendado | 2 = Em Execução | 3 = Cancelado (Quando Atualizado pra Cancelar) | 4 = Finalizado | 5 = Cancelado (Para não repetir info)
    int hora_agendada;  // Das 0h às 23h
    char local_origem[TAM];
    int distancia_pedida; // Valor Fixo da Distancia
    int pid_veiculo;
    int distancia_percorrida_percentagem; // 10% EM 10%
} SERVICO;

typedef struct
{
    int fd;
    pthread_mutex_t *mut;
    int continua;
} TDADOSTHREAD;

// =====================================

// Variaveis Globais Servidor
CLIENTE clientes[NCLIENTES];
SERVICO servicos[NSERVICOS];

int km_total = 0;
int hora_simulada = 0;
int servidor_ativo = 1;
int CLIENTES_ATIVOS = 0;
int SERVICOS_ATIVOS = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_hora = PTHREAD_COND_INITIALIZER;

// =====================================

// Variaveis Globais Cliente
char cli_fifo[30];

// =====================================
// Funções Globais
// 1 = Verdadeiro
// 0 = Falso
// Verifica se o char s é um número
int isNumber(const char *s)
{
    for (int i = 0; s[i]; i++)
        if (!isdigit((unsigned char)s[i]))
            return 0;
    return 1;
}

#endif