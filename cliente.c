// Interação com o utilizador
// Interação com a plataforma/controlador
// Agenda transportes a partir da plataforma/controlador

#include "util.h"

int main(int argc, char *argv[]) {
    int fd, fd_cli;

    char str[20]; 
    PEDIDO_CLIENTE pc;

    // Verifica se o controlador está a funcionar
    if (access(SRV_FIFO, F_OK) != 0)
    {
        printf("[ERRO] O 'controlador' não está a funcionar!\n");
        exit(3);
    };

    // TODO: Necessário identificar o cliente com username
    // Necessario verificar se user foi aceite (se há espaço ou não há repetido)


    // Se sim, abre o FIFO do controlador
    fd = open(SRV_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("[ERRO] Falha ao abrir o FIFO 'controlador'");
        exit(3);
    }

    // Cria o FIFO do cliente
    pc.pid = getpid();
    sprintf(str, CLI_FIFO, pc.pid);
    mkfifo(str, 0640);
    fd_cli = open(str, O_RDWR);

    //AÇÕES DO CLIENTE
    // AGENDAR <HORA> <LOCAL ORIGEM> <DISTANCIA>
    // CONSULTAR
    // CANCELAR <ID>
    // TERMINAR - SAI DA APLICAÇÃO CLIENTE APENAS

    do
    {
        // Pode consultar agendamentos que fez
        // Pode cancelar agendamentos de um serviço previamente agendado, caso ainda não tenha sido realizado

        // Pode fazer um ou vários agendamentos (hora, local origem, distancia de percurso)
            // Envia pedido de agendamento para o controlador
            // Assim que veiculo chegar, indica local de destino
                // Não pode cancelar quando veiculo chegar

    } while (!(strcmp(pc.str, "fim") == 0 || strcmp(pc.str, "sair") == 0));

    close(fd_cli);
    unlink(str);
    close(fd);
    exit(0); // 0 = sucesso
}