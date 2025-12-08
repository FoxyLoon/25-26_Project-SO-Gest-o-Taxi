#include "util.h"

int main(int argc, char *argv[]) {
    int fd, fd_cli;

    char str[20]; 
    PEDIDO_CLIENTE pc;

    // Verifica se o controlador está a funcionar ao
    // verificar se existe o respetivo FIFO
    if (access(SRV_FIFO, F_OK) != 0)
    {
        printf("[ERRO] O 'CONTROLADOR' não está ativo!\n");
        exit(3);
    };

    // Se sim, abre o FIFO do controladorpara escrita
    fd = open(SRV_FIFO, O_WRONLY);

    // Envia o PID do cliente para o controlador
    pc.pid = getpid();
    sprintf(str, CLI_FIFO, pc.pid);
    mkfifo(str, 0640);
    fd_cli = open(str, O_RDWR);

    do
    {
        scanf("%s", pc.str);
    } while (!(strcmp(pc.str, "fim") == 0 || strcmp(pc.str, "sair") == 0));

    return 0;
}