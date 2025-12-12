// Interação com o utilizador
// Interação com a plataforma/controlador
// Agenda transportes a partir da plataforma/controlador

// Pode consultar agendamentos que fez
// Pode cancelar agendamentos de um serviço previamente agendado, caso ainda não tenha sido realizado

// Pode fazer um ou vários agendamentos (hora, local origem, distancia de percurso)
// Envia pedido de agendamento para o controlador
// Assim que veiculo chegar, indica local de destino
// Não pode cancelar quando veiculo chegar

#include "util.h"

// Variaveis Globais
char cli_fifo[30];

void terminarCliente(int s)
{
    unlink(cli_fifo);

    if (s == SIGUSR1)
        {
            printf("SAIDA 1");
        }
        else
        {
            printf("SAIDA 2");
        }

    printf("\n| ============================================ |\n");
    printf("            [ O CLIENTE FOI ENCERRADO ]         \n");
    printf("| ============================================ |\n");
    exit(0);
}

// Está sempre à escuta de mensagens do servidor controlador
void *recebe_msg_servidor(void *arg)
{
    int fd_cli = open(cli_fifo, O_RDONLY | O_NONBLOCK);
    if (fd_cli == -1)
    {
        printf("[CLIENTE] [ERRO] >> Erro ao abrir o fifo do cliente (PID %d)!", getpid());
        exit(1);
    }
    RESPOSTA_SERVER rs;
    while (read(fd_cli, &rs, sizeof(rs)) > 0)
    {
        printf("[SERVIDOR] [MSG] >> %s\n", rs.msg);
        fflush(stdout);
    }

    return NULL;
}

void mostraComandos()
{
    printf("_________________________________________________________________________________\n");
    printf("|                                                                               |\n");
    printf("|	[ COMANDOS DISPONIVEIS ]                [ DESCRIÇÕES ]                      |\n");
    printf("|                                                                               |\n");
    printf("|	> agendar <hora> <origem> <distancia>   » Agenda um servico;                |\n");
    printf("|	> consultar                             » Consulta servicos agendados       |\n");
    printf("|	> cancelar <id_servico>                 » Cancela um servico pelo seu id    |\n");
    printf("|	> terminar                              » Termina a aplicacao do Cliente    |\n");
    printf("|_______________________________________________________________________________|\n");
}

int main(int argc, char *argv[])
{
    // Check Inicial ao iniciar o CLiente
    setbuf(stdout, NULL);

    // Check Inicial ao iniciar o CLiente
    if (argc < 2 || argc > 2)
        return printf("[CLIENTE] [ERRO] >> Uso correto: ./cliente <username>\n");

    // Caso Saida CTRL + C
    signal(SIGINT, terminarCliente);

    // Caso Servidor Termine
    signal(SIGUSR1, terminarCliente);

    // Verifica se o controlador está a funcionar
    if (access(SRV_FIFO, F_OK) != 0)
    {
        printf("[CLIENTE] [ERRO] >> O 'controlador' não está a funcionar!\n");
        exit(1);
    };

    // =====================================

    // Criação do FIFO do cliente
    sprintf(cli_fifo, CLI_FIFO, getpid());
    mkfifo(cli_fifo, 0640);

    // Thread de Escuta do Servidor
    pthread_t thread_1;
    pthread_create(&thread_1, NULL, recebe_msg_servidor, NULL);

    // =====================================

    // Entrada no Servidor
    PEDIDO_CLIENTE pc;
    pc.pid_cliente = getpid();
    strcpy(pc.comando, "login");
    strcpy(pc.username, argv[1]);
    pc.hora = 0;
    pc.distancia = 0;

    // Envia pedido ao Servidor
    int fd_srv = open(SRV_FIFO, O_WRONLY);
    if (fd_srv == -1)
        terminarCliente(0);
    write(fd_srv, &pc, sizeof(pc));
    close(fd_srv);

    // =====================================

    // Apresentação ao utilizador
    printf("\n| ============================================ |\n");
    printf("            [ O CLIENTE FOI INICIADO ]          \n");
    printf("| ============================================ |\n");
    printf("\n| ===== Bem-vindo, %s (PID %d) ===== |\n", pc.username, pc.pid_cliente);
    mostraComandos();
    printf("[CLIENTE] << ");

    // =====================================

    // Input e Validação do Utilizador
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        // =====================================
        // Remove o \n
        buffer[strcspn(buffer, "\n")] = '\0';

        // Extrai o primeiro termo do buffer
        char *cmd = strtok(buffer, " ");

        // =====================================

        // Verifica se cmd o está NULL/VAZIO
        // tolower(cmd); //Necessario verificar se funciona

        if (cmd == NULL)
        {
            printf("[CLIENTE] >> Comando vazio!\n");
            printf("[CLIENTE] << ");
        }

        // =====================================

        else if (!strcmp(cmd, "terminar"))
        {
            char *extra = strtok(NULL, " ");
            if (extra != NULL)
            {
                printf("[CLIENTE] [ERRO] >> '%s' não aceita argumentos.\n", cmd);
                printf("[CLIENTE] << ");
            }
            break;
        }

        // =====================================

        else if (!strcmp(cmd, "agendar"))
        {

            char *hora = strtok(NULL, " ");
            char *origem = strtok(NULL, " ");
            char *dist = strtok(NULL, " ");
            char *extra = strtok(NULL, " ");

            if (!hora || !origem || !dist || extra != NULL)
            {
                printf("[CLIENTE] [ERRO] >> Uso correto: agendar <hora> <origem> <distancia>\n");
                printf("[CLIENTE] << ");
                continue;
            }

            if (!isNumber(hora))
            {
                printf("[CLIENTE] [ERRO] >> A 'Hora' deve ser numérica (0-24).\n");
                printf("[CLIENTE] << ");
                continue;
            }
            if (isNumber(origem))
            {
                printf("[CLIENTE] [ERRO] >> A 'Origem' deve ser uma string.\n");
                printf("[CLIENTE] << ");
                continue;
            }

            if (!isNumber(dist))
            {
                printf("[CLIENTE] [ERRO] >> A 'Distancia' deve ser numérica.\n");
                printf("[CLIENTE] << ");
                continue;
            }

            printf("Comando válido: agendar | Hora = %s | Origem = %s | Distancia = %s\n", hora, origem, dist);

            strcpy(pc.comando, cmd);
            pc.hora = atoi(hora);
            strcpy(pc.origem, origem);
            pc.distancia = atoi(dist);
        }

        // =====================================

        else if (!strcmp(cmd, "consultar"))
        {
            char *extra = strtok(NULL, " ");
            if (extra != NULL)
            {
                printf("[CLIENTE] [ERRO] >> '%s' não aceita argumentos.\n", cmd);
                printf("[CLIENTE] << ");
                continue;
            }

            printf("Comando válido: consultar\n");
            strcpy(pc.comando, cmd);
        }

        // =====================================

        else if (!strcmp(cmd, "cancelar"))
        {

            char *id_servico = strtok(NULL, " ");
            char *extra = strtok(NULL, " ");

            if (id_servico == NULL || extra != NULL)
            {
                printf("[CLIENTE] [ERRO] >> Uso correto: cancelar <id>\n");
                printf("[CLIENTE] << ");
                continue;
            }

            if (!isNumber(id_servico))
            {
                printf("[CLIENTE] [ERRO] >> O 'ID' deve ser numérico.\n");
                printf("[CLIENTE] << ");
                continue;
            }

            printf("Comando válido: cancelar id=%s\n", id_servico);
            strcpy(pc.comando, cmd);
        }

        // =====================================

        else
            printf("[CLIENTE] >> O comando '%s' não existe!\n", cmd);
            printf("[CLIENTE] << ");

        // =====================================

        // Envio do pedido ao controlador
        fd_srv = open(SRV_FIFO, O_WRONLY);
        if (fd_srv == -1)
        {
            printf("[CLIENTE] [ERRO] >> Erro ao abrir o fifo do controlador\n");
            exit(1);
        }
        write(fd_srv, &pc, sizeof(PEDIDO_CLIENTE));
        printf("[SERVIDOR] << [CLIENTE]\nEnviei pedido '%s' ao controlador\n", pc.comando);
        close(fd_srv);

        // =====================================
        
        printf("[CLIENTE] << ");
    }
    terminarCliente(0);
}