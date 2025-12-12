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

    // Entrada no Servidor + Inicialização
    PEDIDO_CLIENTE pc;
    pc.pid_cliente = getpid();
    strcpy(pc.username, argv[1]);
    strcpy(pc.comando, "novo_cliente");
    pc.hora = 0;
    pc.id_servico = 0;
    strcpy(pc.origem, "");
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
    char buffer[128], comando[20], origem[20];
    int arg_hora_id = 0, distancia = 0, comando_valido = 0;
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        // =====================================
        // Remove o \n
        buffer[strcspn(buffer, "\n")] = '\0';

        // Converte para argumentos e guarda argumentos válidos em arg_num
        int arg_num = sscanf(buffer, "%s %d %s %d", comando, &arg_hora_id, origem, &distancia);

        if (arg_num <= 0)
        {
            printf("[CLIENTE] >> Comando vazio!\n");
            comando_valido = 0;
        }

        if (arg_num > 0)
        {
            if (!strcmp(comando, "help")) {
                mostraComandos();
                printf("[CLIENTE] << ");
                comando_valido = 0;
            }

            else if (!strcmp(comando, "terminar"))
            {
                if (arg_num > 1)
                {
                    printf("[CLIENTE] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                    comando_valido = 0;
                }
                else
                {
                    strcpy(pc.comando, comando);
                    comando_valido = 1;
                    break;
                }
            }

            else if (!strcmp(comando, "consultar"))
            {
                if (arg_num > 1)
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

            else if (!strcmp(comando, "agendar"))
            {
                if (arg_num == 1 && strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<hora>' deve ser numérico.\n");
                    comando_valido = 0;
                }
                else if (arg_num == 2 && strcmp(comando, buffer))
                {
                    printf("[CLIENTE] [ERRO] >> O argumento '<origem>' deve ser uma string.\n");
                    comando_valido = 0;
                }
                else if (arg_num == 3 && strcmp(comando, buffer))
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
                else
                {
                    strcpy(pc.comando, comando);
                    pc.hora = arg_hora_id;
                    strcpy(pc.origem, origem);
                    pc.distancia = distancia;
                    comando_valido = 1;
                }
            }
            else
            {
                printf("[CLIENTE] [ERRO] >> Comando '%s' inválido ou desconhecido!\n", comando);
                comando_valido = 0;
            }
        }

        printf("Estrutura: \n");
        printf("Argumentos: %d\n", arg_num);
        printf("Comando: %s\n", pc.comando);
        printf("Hora: %d\n", pc.hora);
        printf("ID: %d\n", pc.id_servico);
        printf("Origem: %s\n", pc.origem);
        printf("Distancia: %d\n", pc.distancia);
        printf("Comando Valido: %d\n", comando_valido);

        // Envio do pedido ao controlador
        if (comando_valido == 1)
        {
            /* fd_srv = open(SRV_FIFO, O_WRONLY);
            if (fd_srv == -1)
            {
                printf("[CLIENTE] [ERRO] >> Erro ao abrir o fifo do controlador\n");
                exit(1);
            }
            write(fd_srv, &pc, sizeof(PEDIDO_CLIENTE)); */
            printf("[SERVIDOR] << [CLIENTE]\nEnviei pedido '%s' ao controlador\n", pc.comando);
            //close(fd_srv);
        }

        // =====================================
        printf("\n[CLIENTE] << ");
    }
    terminarCliente(0);
}