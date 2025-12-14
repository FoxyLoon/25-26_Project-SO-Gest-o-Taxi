#include "util.h"

volatile int servico_cancelado = 0;

void tratar_cancelamento(int sig)
{
    servico_cancelado = 1;
}

void enviar_cliente(int fd_cliente, char *mensagem)
{
    if (fd_cliente != -1)
    {
        RESPOSTA_SERVER r;
        strcpy(r.msg, mensagem);
        write(fd_cliente, &r, sizeof(r));
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s <distancia> <origem> <indice> <pid_cliente>\n", argv[0]);
        exit(1);
    }

    int distancia = atoi(argv[1]);
    char *origem = argv[2];
    int indice = atoi(argv[3]);
    int pid_cliente = atoi(argv[4]);

    // Sinal para cancelamento
    signal(SIGUSR1, tratar_cancelamento);

    // Abre FIFO do cliente
    char fifo_cli[30];
    sprintf(fifo_cli, CLI_FIFO, pid_cliente);
    int fd_cliente = open(fifo_cli, O_WRONLY);
    
    if (fd_cliente == -1)
    {
        perror("Erro ao abrir FIFO do cliente");
        exit(1);
    }

    // Notifica cliente que chegou
    char msg[TAM];
    sprintf(msg, "[VEÍCULO] Cheguei ao local de partida: %s", origem);
    enviar_cliente(fd_cliente, msg);
    
    printf("[VEÍCULO %d] Iniciado - Origem: %s, Distância: %d km\n", 
           getpid(), origem, distancia);

    // Simula viagem (1 km por segundo)
    int distancia_percorrida = 0;
    int checkpoint = 0;

    while (distancia_percorrida < distancia && servico_cancelado == 0)
    {
        sleep(1);
        distancia_percorrida++;

        // A cada 10% de progresso
        int percentagem = (distancia_percorrida * 100) / distancia;
        
        if (percentagem / 10 > checkpoint / 10)
        {
            checkpoint = (percentagem / 10) * 10;

            sprintf(msg, "[VEÍCULO] Progresso: %d%%", checkpoint);
            enviar_cliente(fd_cliente, msg);
            
/*             printf("%d%% ", checkpoint);
 */            fflush(stdout);
        }
    }

    // Finaliza serviço
    if (servico_cancelado)
    {
        sprintf(msg, "[VEÍCULO] Serviço cancelado pelo administrador");
        enviar_cliente(fd_cliente, msg);
        printf("\n[VEÍCULO %d] Serviço cancelado\n", getpid());
    }
    else
    {
        sprintf(msg, "[VEÍCULO] Serviço concluído - Chegou ao destino!");
        enviar_cliente(fd_cliente, msg);
        printf("\n[VEÍCULO %d] Serviço concluído\n", getpid());
    }

    close(fd_cliente);
    exit(0);
}