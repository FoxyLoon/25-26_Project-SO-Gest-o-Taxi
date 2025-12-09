// Interação com o utilizador
// Interação com a plataforma/controlador
// Agenda transportes a partir da plataforma/controlador

#include "util.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[Falha ao iniciar] Uso correto: ./cliente <username>\n");
        exit(1);
    }

    int fd, fd_cli;
    char str[20];
    PEDIDO_CLIENTE pc;
    RESPOSTA_SERVER rs;

    // Verifica se o controlador está a funcionar
    if (access(SRV_FIFO, F_OK) != 0)
    {
        printf("[ERRO] O 'controlador' não está a funcionar!\n");
        exit(3);
    };

    // Se sim, abre o FIFO do controlador
    fd = open(SRV_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("[ERRO] Falha ao abrir o FIFO 'controlador'");
        exit(3);
    }

    printf("Registo Inicial:\n");

    // Inicio de sessão + guarda nome na estrutura
    strcpy(pc.username, argv[1]);
    printf("Username: %s\n", pc.username);

    // Cria o FIFO do cliente
    pc.id = getpid();
    printf("ID: %d\n", pc.id);

    sprintf(str, CLI_FIFO, pc.id);
    mkfifo(str, 0640);
    fd_cli = open(str, O_RDWR);

    // AÇÕES DO CLIENTE
    //  AGENDAR <HORA> <LOCAL ORIGEM> <DISTANCIA>
    //  CONSULTAR
    //  CANCELAR <ID>
    //  TERMINAR - SAI DA APLICAÇÃO CLIENTE APENAS

    //Verifica comando introduzido
    do
    {
        printf("Comandos disponiveis:\n");
        printf("\t- AGENDAR <HORA> <LOCAL ORIGEM> <DISTANCIA>\n");
        printf("\t- CONSULTAR\n");
        printf("\t- CANCELAR <ID>\n");
        printf("\t- TERMINAR\n");

        printf("\nDigite o comando: ");
        scanf("%s %d %s %d", pc.comando, &pc.hora, pc.origem, &pc.distancia);

        if (strcmp(pc.comando, "agendar") == 0)
        {
            write(fd, &pc, sizeof(PEDIDO_CLIENTE));
        }
        else if (strcmp(pc.comando, "consultar") == 0)
        {
            write(fd, &pc, sizeof(PEDIDO_CLIENTE));
            printf("%s\n", rs.msg);
        }
        else if (strcmp(pc.comando, "cancelar") == 0)
        {
            write(fd, &pc, sizeof(PEDIDO_CLIENTE));
        }
        else if (strcmp(pc.comando, "terminar") == 0)
        {
            write(fd, &pc, sizeof(PEDIDO_CLIENTE));
            break;
        }
        else
        {
            printf("Comando invalido!\n");
        }
    } while (!(strcmp(pc.comando, "terminar") == 0));

    // Pode consultar agendamentos que fez
    // Pode cancelar agendamentos de um serviço previamente agendado, caso ainda não tenha sido realizado

    // Pode fazer um ou vários agendamentos (hora, local origem, distancia de percurso)
    // Envia pedido de agendamento para o controlador
    // Assim que veiculo chegar, indica local de destino
    // Não pode cancelar quando veiculo chegar

    close(fd_cli);
    unlink(str);
    close(fd);
    exit(0); // 0 = sucesso
}