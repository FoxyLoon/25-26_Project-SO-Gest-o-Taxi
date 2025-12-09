// Gestão dos veiculos
// Recebe Pedidios do Cliente
// Prepara os veiculos
// Contabiliza KM totais percorridos por todos os automoveis


#include "util.h"

int main(int argc, char *argv[]) {

    // TODO: Maximo users = 30

    int fd;
    PEDIDO_CLIENTE pc;

    // Cria o FIFO do controlador
    mkfifo(SRV_FIFO, 0640);
    fd = open(SRV_FIFO, O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir o FIFO 'controlador'");
        exit(1);
    }

    

    // AÇÕES DO CONTROLADOR:
    // LISTAR - MOSTRA TODOS OS SERVIÇOS AGENDADOS
    // UTILIZ - MOSTRA TODOS OS UTILIZADORES LIGAD1OS
    // FROTA - MOSTRA AS PERCENTAGENS DAS VIAGENS
    // CANCELAR <ID> - CANCELA UM SERVIÇO ATRAVES DO SEU ID (AGENDADO OU A SER REALIZADO)
        // SE ID = 0, CANCELA TODOS OS SERVIÇOS
    
    // KM - MOSTRA OS KILOMETROS PERCORRIDOS
    // HORA - MOSTRA A HORA SIMULADA
    // TERMINAR - TERMINA O SIMULADOR

    do {
        // Recebe o pedido de agendamento cliente (cliente_pid, hora, local origem, distancia)
        // Envia veiculo para o cliente à "hora pretendida"
        // Envia informações do veiculo para o cliente a informar que entrou, andou e chegou ao destino
        // ATENÇÃO QUE O VEICULO.C TEM QUE ESTAR ENVOLVIDO!
        read(fd, &pc, sizeof(PEDIDO_CLIENTE));
        printf("Comando: %s\n", pc.comando);

        
    } while (!(strcmp(pc.comando, "TERMINAR") == 0));

    close(fd);
    unlink(SRV_FIFO);
    exit(0); // 0 = sucesso
}