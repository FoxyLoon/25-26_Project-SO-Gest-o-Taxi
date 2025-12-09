// Gestão dos veiculos
// Recebe Pedidios do Cliente
// Prepara os veiculos
// Contabiliza KM totais percorridos por todos os automoveis

#include "util.h"

int main(int argc, char *argv[])
{
    int n, i, fd, fd_cli;
    int pos, livre;
    char str_pid[20];

    PEDIDO_CLIENTE pc;
    RESPOSTA_SERVER rs;

    // =====================================

    // Cria o FIFO do controlador
    mkfifo(SRV_FIFO, 0640);
    fd = open(SRV_FIFO, O_RDWR);
    if (fd == -1)
    {
        printf("Erro ao abrir o FIFO 'controlador'");
        return 1;
    }

    // =====================================

    // Cria o array de utilizadores
    int clientes[NCLIENTES];
    for (i = 0; i < NCLIENTES; i++)
    {
        clientes[i] = -1;
    }

    // =====================================

    printf("O 'controlador' está a funcionar!\n");

    // =====================================

    // Fica à espera de pedidos
    do
    {
        n = read(fd, &pc, sizeof(PEDIDO_CLIENTE));
        if (n == -1)
        {
            printf("Erro ao ler do FIFO 'controlador'");
            return 1;
        }
        if (n == sizeof(PEDIDO_CLIENTE))
        {
            printf("Recebi %d bytes... do cliente %d\n", n, pc.pid);

            // =====================================

            // Verifica se existe espaço para o utilizador ou se ele ja existe
            for (pos = -1, livre = -1, i = 0; i < NCLIENTES; i++)
            {
                if (clientes[i] == pc.pid)
                    pos = i;
                if (livre == -1 && clientes[i] == -1)
                    livre = i;
            }
            if (pos == -1 && livre != -1)
            {
                pos = livre;
                clientes[pos] = pc.pid;
            }
            if (pos != -1 && (strcmp(pc.comando, "terminar") == 0))
            {
                clientes[pos] = -1;
            }

            // =====================================

            for (i = 0; i < NCLIENTES; i++)
            {
                if (clientes[i] != -1)
                {
                    // Verifica comando executado
                    if (strcmp(pc.comando, "agendar") == 0)
                    {
                        printf("Comando Recebido: %s\n", pc.comando);
                        printf("Hora Recebida: %d\n", pc.hora);
                        printf("Origem Recebida: %s\n", pc.origem);
                        printf("Distancia Recebida: %d\n", pc.distancia);

                        // Envia a resposta ao cliente
                        snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] AGENDAMENTO RECEBIDO");
                    }
                    if (strcmp(pc.comando, "consultar") == 0)
                    {
                        printf("Comando Recebido: %s\n", pc.comando);

                        // Envia a resposta ao cliente
                        snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] CONSULTA RECEBIDA");
                    }
                    if (strcmp(pc.comando, "cancelar") == 0)
                    {
                        printf("Comando Recebido: %s\n", pc.comando);
                        printf("ID Recebido: %d\n", pc.id_servico);

                        // Envia a resposta ao cliente
                        snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] CANCELAMENTO RECEBIDO");
                    }
                    // Será removido porque o cliente não é suposto terminar o servidor
                    // Apenas está para fins de teste!
                    if (strcmp(pc.comando, "terminar") == 0)
                    {
                        printf("Comando Recebido: %s\n", pc.comando);

                        // Envia a resposta ao cliente
                        snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] TERMINAR RECEBIDO");                        
                    }

                }
            }


            // Envia comando 
            for (i = 0; i < NCLIENTES; i++)
            {
                if (clientes[i] != -1)
                {
                    // No str_pid coloca o PID do utilizador formatado
                    sprintf(str_pid, CLI_FIFO, clientes[i]);
                    // Abre o FIFO especifico do cliente
                    fd_cli = open(str_pid, O_WRONLY);
                    
                    if (fd_cli < 0)
                    {
                        printf("Erro ao abrir o FIFO 'controlador'");
                        continue;
                    }
                    n = write(fd_cli, &rs, sizeof(RESPOSTA_SERVER));
                    printf("Comando Enviado: %s\n", rs.msg);

                    // Fecha o FIFO do cliente
                    close(fd_cli);
                }
            }
        }

        // =====================================

        // Mostra os utilizadores ativos
        // printf("\nUTILIZADORES ATIVOS:\n");
        // for (i = 0; i < NCLIENTES; i++)
        // {
        //     printf("CLIENTES[%d] = %d\n", i, clientes[i]);
        // }
    } while (strcmp(pc.comando, "terminar") != 0);

    close(fd);
    unlink(SRV_FIFO);
    return 0;

    // =====================================
    // LIXO QUE PODE SER USADO
    // =====================================

    // // TODO: Maximo users = 30
    // PEDIDO_CLIENTE pc;

    // mkfifo(SRV_FIFO, 0640);
    // int fd = open(SRV_FIFO, O_RDWR);
    // if (fd == -1) {
    //     printf("Erro ao abrir o FIFO 'controlador'");
    //     return 1;
    // }

    // int n = read(fd, &pc, sizeof(PEDIDO_CLIENTE));
    // if (n == sizeof(PEDIDO_CLIENTE)) {
    //     printf("Comando Recebido: %s\n", pc.comando);
    // }

    // close(fd);
    // unlink(SRV_FIFO);

    // return 0;

    // int fd;
    // PEDIDO_CLIENTE pc;

    // // Cria o FIFO do controlador
    // mkfifo(SRV_FIFO, 0640);
    // fd = open(SRV_FIFO, O_RDWR);
    // if (fd == -1) {
    //     printf("Erro ao abrir o FIFO 'controlador'");
    //     exit(1);
    // }

    // // AÇÕES DO CONTROLADOR:
    // // LISTAR - MOSTRA TODOS OS SERVIÇOS AGENDADOS
    // // UTILIZ - MOSTRA TODOS OS UTILIZADORES LIGAD1OS
    // // FROTA - MOSTRA AS PERCENTAGENS DAS VIAGENS
    // // CANCELAR <ID> - CANCELA UM SERVIÇO ATRAVES DO SEU ID (AGENDADO OU A SER REALIZADO)
    //     // SE ID = 0, CANCELA TODOS OS SERVIÇOS

    // // KM - MOSTRA OS KILOMETROS PERCORRIDOS
    // // HORA - MOSTRA A HORA SIMULADA
    // // TERMINAR - TERMINA O SIMULADOR

    // do {
    //     // Recebe o pedido de agendamento cliente (cliente_pid, hora, local origem, distancia)
    //     // Envia veiculo para o cliente à "hora pretendida"
    //     // Envia informações do veiculo para o cliente a informar que entrou, andou e chegou ao destino
    //     // ATENÇÃO QUE O VEICULO.C TEM QUE ESTAR ENVOLVIDO!
    //     read(fd, &pc, sizeof(PEDIDO_CLIENTE));
    //     printf("Comando: %s\n", pc.comando);
    //     write(fd, &pc, sizeof(PEDIDO_CLIENTE));

    // } while (!(strcmp(pc.comando, "TERMINAR") == 0));

    // close(fd);
    // unlink(SRV_FIFO);
    // exit(0); // 0 = sucesso
}
