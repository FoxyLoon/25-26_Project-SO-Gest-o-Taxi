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
#define SRV_FIFO "controlador"
#define CLI_FIFO "cli_%d"

typedef struct {
    
    
    char str[TAM];
    int pid;
} PEDIDO_CLIENTE;

typedef struct {
    char str[TAM];
} RESPOSTA_SERVER;