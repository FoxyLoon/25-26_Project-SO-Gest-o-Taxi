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

#define NVEICULOS 10

#define TAM 100
#define TAM_USERNAME 30

#define SRV_FIFO "canal"
#define CLI_FIFO "cli_%d"

typedef struct {
    int pid;
    int id_servico;
    char comando[TAM];
    char username[TAM_USERNAME];
    int hora;
    char origem[TAM];
    int distancia;
    char destino[TAM];
} PEDIDO_CLIENTE;

typedef struct {
    char msg[TAM];
} RESPOSTA_SERVER;

// Verifica se o char s é um número
int isNumber(const char *s) {
    for (int i = 0; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}