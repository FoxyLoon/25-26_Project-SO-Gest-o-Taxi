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
// Está sempre à escuta de mensagens do servidor controlador
void *recebe_msg_servidor(void *arg)
{
    TDADOSTHREAD *tdt_cli = (TDADOSTHREAD *)arg;
    RESPOSTA_SERVER rs;
    ssize_t n;
    while ((n = read(tdt_cli->fd, &rs, sizeof(rs))) > 0)
    {
        if (n == sizeof(rs))
        {
            printf("\n[SERVIDOR] [INFO] >> %s\n", rs.msg);
            if (strcmp(rs.msg, "O Servidor está cheio!"))
            {
                printf("[CLIENTE] << ");
            }
            fflush(stdout);
        }
    }
    printf("[SERVIDOR] >> FIFO fechado\n");

    return NULL;
}

/* void enviar_pedido_servidor(PEDIDO_CLIENTE pc)
{
    printf("Username: %s\n", pc.username);
    printf("Comando: %s\n", pc.comando);
    printf("PID: %d\n", pc.pid_cliente);
    printf("hora: %d\n", pc.hora);
    printf("origem: %s\n", pc.origem);
    printf("distancia: %d\n", pc.distancia);
    printf("id_servico: %d\n", pc.id_servico);
    int fd_srv = open(SRV_FIFO, O_WRONLY);
    if (fd_srv == -1)
    {
        printf("[CLIENTE] [ERRO] >> Erro ao abrir o fifo do controlador\n");
        exit(1);
    }
    write(fd_srv, &pc, sizeof(PEDIDO_CLIENTE));
    printf("[SERVIDOR] << [CLIENTE] << Enviei pedido '%s' ao controlador\n", pc.comando);
    close(fd_srv);
} */

void terminarCliente(int s)
{
    unlink(cli_fifo);

    if (s == SIGUSR1)
    {
        printf("\n[SERVIDOR] >> O servidor encerrou!\n");
    }
    else if (s == SIGINT)
    {
        /* PEDIDO_CLIENTE pc;
        strcpy(pc.comando, "terminar");
        pc.pid_cliente = getpid();
        enviar_pedido_servidor(pc); */
        printf("\n\n[CLIENTE] >> O cliente encerrou forçadamente!\n");
    }
    else
    {
        printf("\n[CLIENTE] >> O cliente encerrou!\n");
    }

    printf("\n| ============================================ |\n");
    printf("            [ O CLIENTE FOI ENCERRADO ]         \n");
    printf("| ============================================ |\n");
    exit(0);
}
// =====================================

void mostraComandos()
{
    printf("________________________________________________________________________\n\n");
    printf("[ COMANDOS DISPONIVEIS ]                [ DESCRIÇÕES ]\n\n");
    printf("> ajuda                                 » Mostra este painel de comandos\n");
    printf("> agendar <hora> <origem> <distancia>   » Agenda um servico\n");
    printf("> consultar                             » Consulta servicos agendados\n");
    printf("> cancelar <id_servico>                 » Cancela um servico pelo seu id\n");
    printf("> terminar                              » Termina a aplicacao do Cliente\n");
    printf("________________________________________________________________________\n");
}

// =====================================

int main(int argc, char *argv[])
{
    // =====================================

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

    TDADOSTHREAD dt;
    dt.fd = open(cli_fifo, O_RDWR);
    dt.continua = 1;

    if (dt.fd == -1)
    {
        perror("[CLIENTE] Erro ao abrir FIFO do cliente");
        exit(1);
    }

    // Thread de Escuta do Servidor
    pthread_t thread_1;
    pthread_create(&thread_1, NULL, recebe_msg_servidor, &dt);

    // =====================================

    // Entrada no Servidor + Inicialização
    PEDIDO_CLIENTE pc;
    pc.pid_cliente = getpid();
    strcpy(pc.username, argv[1]);
    strcpy(pc.comando, "novo_cliente");
    pc.hora = 0;
    pc.id_servico = 0;
    strcpy(pc.origem, "");
    pc.distancia = 0;

    // Envia notificação ao Servidor
    int fd_srv = open(SRV_FIFO, O_WRONLY);
    if (fd_srv == -1)
        terminarCliente(0);
    write(fd_srv, &pc, sizeof(pc));
    close(fd_srv);

    // =====================================

    // Apresentação ao utilizador
    printf("| ============================================ |\n");
    printf("            [ O CLIENTE FOI INICIADO ]        \n");
    printf("| ============================================ |\n");
    printf("\n- Bem-vindo, %s (PID %d)\n", pc.username, pc.pid_cliente);
    printf("- Dica: use o comando 'ajuda'\n");

    // =====================================

    // Input e Validação do Utilizador
    char buffer[128], comando[20], origem[20], extra[20];
    int arg_hora_id = 0, distancia = 0, comando_valido = 0;
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        // =====================================
        // Remove o \n
        buffer[strcspn(buffer, "\n")] = '\0';

        // Converte para argumentos e guarda argumentos válidos em arg_num
        int arg_num = sscanf(buffer, "%s %d %s %d %s", comando, &arg_hora_id, origem, &distancia, extra);

        // =====================================

        if (arg_num <= 0)
        {
            printf("[CLIENTE] >> Comando vazio!\n");
            comando_valido = 0;
        }

        if (arg_num > 0)
        {
            if (!strcmp(comando, "ajuda"))
            {
                mostraComandos();
                comando_valido = 0;
            }

            // =====================================

            else if (!strcmp(comando, "terminar"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                    comando_valido = 0;
                }
                else
                {
                    strcpy(pc.comando, comando);

                    // Envia notificação ao Servidor
                    fd_srv = open(SRV_FIFO, O_WRONLY);
                    if (fd_srv == -1)
                        terminarCliente(0);
                    write(fd_srv, &pc, sizeof(pc));
                    close(fd_srv);

                    comando_valido = 1;
                    break;
                }
            }

            // =====================================

            else if (!strcmp(comando, "consultar"))
            {
                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                    comando_valido = 0;
                }
                else
                {
                    strcpy(pc.comando, comando);
                    comando_valido = 1;
                }
            }

            // =====================================

            else if (!strcmp(comando, "cancelar"))
            {
                if (arg_num == 1 && strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<id>' deve ser numérico.\n");
                    comando_valido = 0;
                }
                else if (arg_num > 2)
                {
                    printf("[CLIENTE] [ERRO] >> '%s' tem demasiados argumentos!\n", comando);
                    comando_valido = 0;
                }
                else if (arg_num < 2)
                {
                    printf("[CLIENTE] [ERRO] >> Falta argumento: '<id>'!\n");
                    comando_valido = 0;
                }
                else
                {
                    strcpy(pc.comando, comando);
                    pc.id_servico = arg_hora_id;
                    comando_valido = 1;
                }
            }

            // =====================================

            else if (!strcmp(comando, "agendar"))
            {
                if (arg_num == 1 && strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<hora>' deve ser numérico.\n");
                    comando_valido = 0;
                }
                else if ((arg_num == 3 || arg_num == 4) && isNumber(origem))
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<origem>' deve ser uma string.\n");
                    comando_valido = 0;
                }
                else if (arg_num == 3)
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<distancia>' deve ser numérico.\n");
                    comando_valido = 0;
                }
                else if (arg_num > 4)
                {
                    printf("[CLIENTE] [ERRO] >> '%s' tem demasiados argumentos!\n", comando);
                    comando_valido = 0;
                }
                else if (arg_num < 2)
                {
                    printf("[CLIENTE] [ERRO] >> Faltam argumentos: '<hora>', '<origem>' e '<distancia>'!\n");
                    comando_valido = 0;
                }
                else if (arg_num < 3)
                {
                    printf("[CLIENTE] [ERRO] >> Faltam argumentos: '<origem>' e '<distancia>'!\n");
                    comando_valido = 0;
                }
                else if (arg_num < 4)
                {
                    printf("[CLIENTE] [ERRO] >> Falta argumento: '<distancia>'!\n");
                    comando_valido = 0;
                }
                else if (arg_hora_id < 0 && arg_hora_id >= 24)
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<hora>' deve estar entre 0 e 23, pois correspondem à hora do dia.\n");
                    comando_valido = 0;
                }
                else
                {
                    strcpy(pc.comando, comando);
                    pc.hora = arg_hora_id;
                    strcpy(pc.origem, origem);
                    pc.distancia = distancia;
                    comando_valido = 1;
                }
            }

            // =====================================

            else
            {
                printf("[CLIENTE] [ERRO] >> Comando '%s' inválido ou desconhecido!\n", comando);
                comando_valido = 0;
            }
        }

        // =====================================

        // Envio do pedido ao controlador
        if (comando_valido == 1)
        {
            int fd_srv = open(SRV_FIFO, O_WRONLY);
            if (fd_srv == -1)
            {
                printf("[CLIENTE] [ERRO] >> Erro ao abrir o fifo do controlador\n");
                exit(1);
            }
            if (write(fd_srv, &pc, sizeof(PEDIDO_CLIENTE)) == -1)
            {
                printf("[CLIENTE] [ERRO] >> Erro ao enviar o pedido ao controlador\n");
            }
            else
            {
                printf("[SERVIDOR] << [CLIENTE] << Enviei pedido '%s' ao controlador\n", pc.comando);
            }
            close(fd_srv);
            /* enviar_pedido_servidor(pc); */
        }

        // =====================================

        printf("\n[CLIENTE] << ");
    }
    terminarCliente(0);
}