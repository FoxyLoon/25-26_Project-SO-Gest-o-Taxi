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
    printf("_________________________________________________\n");
    printf("|												|\n");
    printf("|		----- COMANDOS DISPONIVEIS -----		|\n");
    printf("|												|\n");
    printf("|	> agendar <hora> <origem> <distancia>		|\n");
    printf("|		» Agenda um servico;					|\n");
    printf("|												|\n");
    printf("|	> consultar									|\n");
    printf("|		» Consulta servicos agendados			|\n");
    printf("|												|\n");
    printf("|	> cancelar <id_servico>						|\n");
    printf("|		» Cancela um servico pelo seu id		|\n");
    printf("|												|\n");
    printf("|	> terminar									|\n");
    printf("|		» Termina a aplicacao do Cliente		|\n");
    printf("|_______________________________________________|");
    fflush(stdout);

    return NULL;
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
        printf("[ERRO] O 'controlador' não está a funcionar!\n");
        exit(1);
    };

    // =====================================

    // Definicoes de estruturas

    SERVICO svc;
    int n;

    // =====================================

    // Abre o FIFO do controlador
    int fd_srv = open(SRV_FIFO, O_WRONLY);
    if (fd_srv == -1)
    {
        printf("Erro ao abrir o FIFO 'controlador'");
        exit(1);
    }

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
    strcpy(pc.pid_cliente, getpid());
    strcpy(pc.comando, "login");
    strcpy(pc.username, argv[1]);
    strcpy(pc.hora, 0);
    strcpy(pc.distancia, 0);
    fd_srv = open(SRV_FIFO, O_WRONLY);
    if (fd_srv == -1)
        sair(0);
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
        //tolower(cmd); //Necessario verificar se funciona
        if(!strcmp(cmd, "terminar")) {
            break;
        } else if (!strcmp(cmd, NULL))
        {
            printf("[CLIENTE] >> Comando vazio!\n");
            continue;
        } else {
            printf("[CLIENTE] >> Comando '%s' inválido!\n", cmd);
        }
    }
    terminar_cliente(0);

    // Validação de Input
    /* do
    {

        FD_ZERO(&fds);
        FD_SET(0, &fds);      // teclado
        FD_SET(fd_cli, &fds); // fifo (fd_cli)
        printf("CMD >> ");

        int max_fd = fd_cli > 0 ? fd_cli : 0;
        res = select(fd_cli + 1, &fds, NULL, NULL, NULL);
        if (res > 0)
        {
            if (FD_ISSET(0, &fds))
            {

                fflush(stdout);
                fflush(stdin);

                // Input do Utilizador
                if (!fgets(buffer, sizeof(buffer), stdin))
                {
                    printf("Erro ao ler input.\n");
                    continue;
                }

                // =====================================

                // =====================================

                if (strcmp(cmd, "agendar") == 0)
                {

                    char *hora = strtok(NULL, " ");
                    char *origem = strtok(NULL, " ");
                    char *dist = strtok(NULL, " ");
                    char *extra = strtok(NULL, " ");

                    if (!hora || !origem || !dist || extra != NULL)
                    {
                        printf("Uso correto: agendar <hora> <origem> <distancia>\n");
                        continue;
                    }

                    if (!isNumber(hora))
                    {
                        printf("[ERRO] hora deve ser numérica.\n");
                        continue;
                    }
                    if (!isNumber(dist))
                    {
                        printf("[ERRO] distancia deve ser numérica.\n");
                        continue;
                    }

                    printf("Comando válido: agendar | Hora = %s | Origem = %s | Distancia = %s\n", hora, origem, dist);

                    strcpy(pc.comando, cmd);
                    pc.hora = atoi(hora);
                    strcpy(pc.origem, origem);
                    pc.distancia = atoi(dist);
                }

                // =====================================

                else if (strcmp(cmd, "consultar") == 0)
                {
                    char *extra = strtok(NULL, " ");
                    if (extra != NULL)
                    {
                        printf("[ERRO] 'consultar' não aceita argumentos.\n");
                        continue;
                    }

                    printf("Comando válido: consultar\n");
                    strcpy(pc.comando, cmd);
                }

                // =====================================

                else if (strcmp(cmd, "cancelar") == 0)
                {

                    char *id_servico = strtok(NULL, " ");
                    char *extra = strtok(NULL, " ");

                    if (id_servico == NULL || extra != NULL)
                    {
                        printf("Uso correto: cancelar <id>\n");
                        continue;
                    }

                    if (!isNumber(id_servico))
                    {
                        printf("[ERRO] id deve ser numérico.\n");
                        continue;
                    }

                    printf("Comando válido: cancelar id=%s\n", id_servico);
                    strcpy(pc.comando, cmd);

                    // Necessario reformular por causa do array de servicos
                    // svc.id_servico = atoi(id_servico);
                }

                // =====================================

                else if (strcmp(cmd, "terminar") == 0)
                {

                    char *extra = strtok(NULL, " ");
                    if (extra != NULL)
                    {
                        printf("[ERRO] 'terminar' não aceita argumentos.\n");
                        continue;
                    };

                    strcpy(pc.comando, cmd);

                    printf("A terminar...\n");
                }

                // =====================================

                else
                {
                    printf("Comando inválido: %s\n", cmd);
                    continue;
                }

                // Enviar pedido ao controlador
                if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1)
                {
                    printf("[ERRO] Falha ao escrever no FIFO 'canal'");
                    return 1;
                };
                printf("Enviei pedido '%s' ao controlador\n", pc.comando);
            }
            if (FD_ISSET(fd_cli, &fds))
            {
                if (read(fd_cli, &rs, sizeof(RESPOSTA_SERVER)) == -1)
                {
                    printf("[ERRO] Falha ao ler 'canal'");
                    return 1;
                };
                printf(" %s\n", rs.msg);
            }
        }

    } while (strcmp(pc.comando, "terminar") != 0);

    close(fd_cli);
    close(fd_srv); */

    exit(0);
}
