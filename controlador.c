// Gestão dos veiculos
// Recebe Pedidios do Cliente
// Prepara os veiculos
// Contabiliza KM totais percorridos por todos os automoveis


#include "util.h"

int main(int argc, char *argv[]) {

    // int arr[5];
    // int fd = open(SRV_FIFO, O_RDONLY);
    // if (fd == -1) {
    //     printf("Erro ao abrir o FIFO 'controlador'");
    //     return 1;
    // }

    // int i;
    // for (i=0; i<5; i++) {
    //     if (read(fd, &arr[i], sizeof(int)) == -1) {
    //         printf("Erro ao ler do FIFO 'controlador'");
    //         return 1;
    //     };
    //     printf("Lido: %d\n", arr[i]);
    // }
    // close(fd);
    // return 0;
    PEDIDO_CLIENTE pc;

    mkfifo(SRV_FIFO, 0640);
    int fd = open(SRV_FIFO, O_RDWR);
    if (fd == -1) {
        printf("Erro ao abrir o FIFO 'controlador'");
        return 1;
    }

    printf("O 'controlador' está a funcionar!\n");

    do {
        if (read(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
            printf("Erro ao ler do FIFO 'controlador'");
            return 1;
        }
        if (strcmp(pc.comando, "agendar") == 0) {
            printf("Comando Recebido: %s\n", pc.comando);
            printf("Hora Recebida: %d\n", pc.hora);
            printf("Origem Recebida: %s\n", pc.origem);
            printf("Distancia Recebida: %d\n", pc.distancia);
        }
        else if (strcmp(pc.comando, "consultar") == 0) {
            printf("Comando Recebido: %s\n", pc.comando);
        }
        else if (strcmp(pc.comando, "cancelar") == 0) {
            printf("Comando Recebido: %s\n", pc.comando);
            printf("ID Recebido: %d\n", pc.id_servico);
        }
        // Será removido porque o cliente não é suposto terminar o servidor
        // Apenas está para fins de teste!
        else if (strcmp(pc.comando, "terminar") == 0) {
            printf("Comando Recebido: %s\n", pc.comando);
        }
    } while(strcmp(pc.comando, "terminar") != 0);

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