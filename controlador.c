// Gestão dos veiculos
// Recebe Pedidios do Cliente
// Prepara os veiculos
// Contabiliza KM totais percorridos por todos os automoveis

// AÇÕES DO CONTROLADOR:
// LISTAR - MOSTRA TODOS OS SERVIÇOS AGENDADOS
// UTILIZ - MOSTRA TODOS OS UTILIZADORES LIGAD1OS
// FROTA - MOSTRA AS PERCENTAGENS DAS VIAGENS
// CANCELAR <ID> - CANCELA UM SERVIÇO ATRAVES DO SEU ID (AGENDADO OU A SER REALIZADO)
// SE ID = 0, CANCELA TODOS OS SERVIÇOS

// KM - MOSTRA OS KILOMETROS PERCORRIDOS
// HORA - MOSTRA A HORA SIMULADA
// TERMINAR - TERMINA O SIMULADOR

#include "util.h"

void *tempo(void *arg)
{
    // fazer com sleep
    // lança thread para o veiculo (thread dentro de thread)
    printf("Ola thread1\n");
    return NULL;
}

// serao 10 veiculos
void *veiculo(void *arg)
{
}

// Processo comandos do cliente
void *cliente(void *arg)
{
    pthread_mutex_lock(&mutex);

    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Processa comandos do administrador
void *administrador(void *arg)
{
    pthread_mutex_lock(&mutex);

    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Envia repostas ao respetivo cliente pelo pid do cliente
void envia_msg_cliente(int pid_cli, char *mensagem)
{
    char fd_cli[20];
    sprintf(fd_cli, CLI_FIFO, pid_cli);
    int fd = open(fd_cli, O_WRONLY);
    if (fd != -1)
    {
        RESPOSTA_SERVER rs;
        strcpy(rs.msg, mensagem);
        write(fd, &rs, sizeof(RESPOSTA_SERVER));
        close(fd);
    }
}

void terminarServidor(int s)
{

    unlink(SRV_FIFO);
    servidor_ativo = 0;
    int i;
    for (i = 0; i < SERVICOS_ATIVOS; i++)
        if (servicos[i].status_servico == 1)
            kill(servicos[i].pid_veiculo, SIGKILL);
    for (i = 0; i < CLIENTES_ATIVOS; i++)
        if (clientes[i].pid_cliente)
            kill(clientes[i].pid_cliente, SIGUSR1);
    printf("\n| ======================================== |\n");
    printf("        [ O SERVIDOR FOI ENCERRADO ]        \n");
    printf("| ======================================== |\n");
    exit(0);
}

void mostraComandosServidor()
{
    printf("_________________________________________________________________________________________\n");
    printf("|                                                                                       |\n");
    printf("|   [ COMANDOS DISPONIVEIS ]    [ DESCRIÇÕES ]                                          |\n");
    printf("|                                                                                       |\n");
    printf("|   > ajuda                     » Mostra este painel de comandos;                       |\n");
    printf("|   > listar                    » Mostra todos os servicos agendados                    |\n");
    printf("|   > utiliz                    » Mostra utilizadores conectados                        |\n");
    printf("|   > frota                     » Mostra percentagem de viagem dos veiculos             |\n");
    printf("|   > cancelar <id_servico>     » Cancela um servico pelo seu id (0 = todos)            |\n");
    printf("|   > km                        » Mostra total de km percorridos por todos os veiculos  |\n");
    printf("|   > hora                      » Mostra a hora simulada atual do servidor              |\n");
    printf("|   > terminar                  » Termina o Servidor e todos os Clientes ligados        |\n");
    printf("|_______________________________________________________________________________________|\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, terminarServidor);
    int n, i, fd, fd_cli, idx;
    int posicao_pid, livre;
    char str_pid[20];

    PEDIDO_CLIENTE pc;
    RESPOSTA_SERVER rs;

    // =====================================

    // Cria o FIFO do servidor controlador
    if (mkfifo(SRV_FIFO, 0640) == -1)
    {
        printf("[SERVIDOR] [ERRO] >> Erro ao criar o fifo do controlador.\n");
        exit(1);
    }
    fd = open(SRV_FIFO, O_RDWR);
    if (fd == -1)
    {
        printf("[SERVIDOR] [ERRO] >> Erro ao abrir o fifo do controlador");
        exit(1);
    }

    // =====================================

    pthread_mutex_init(&mutex, NULL);

    pthread_t thread_1, thread_2, thread_3;
    pthread_create(&thread_1, NULL, tempo, NULL);
    pthread_create(&thread_2, NULL, cliente, NULL);
    pthread_create(&thread_3, NULL, administrador, NULL);

    // =====================================

    // Apresentação ao administrador
    printf("\n| ============================================ |\n");
    printf("            [ O SERVIDOR FOI INICIADO ]         \n");
    printf("| ============================================ |\n");
    printf("\n| ===== Bem-vindo, Administrador (PID %d) ===== |\n", getpid());
    
    mostraComandosServidor();
    printf("[SERVIDOR] << ");

    // =====================================

    // Input e Validação do Administraador
    char buffer[128], comando[20], extra[20];
    int id = 0/* , distancia = 0, comando_valido = 0 */;
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        // =====================================
        // Remove o \n
        buffer[strcspn(buffer, "\n")] = '\0';

        // Converte para argumentos e guarda argumentos válidos em arg_num
        int arg_num = sscanf(buffer, "%s %d %s", comando, &id, extra);

        // =====================================

        if (arg_num <= 0)
        {
            printf("[SERVIDOR] >> Comando vazio!\n");
        }

        if (arg_num > 0)
        {
            if (!strcmp(comando, "ajuda"))
                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    int number = strcmp(comando, buffer);
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    mostraComandosServidor();
                    printf("[SERVIDOR] << ");
                }

            // =====================================

            else if (!strcmp(comando, "terminar"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    strcpy(pc.comando, comando);
                    break;
                }
            }

            // =====================================

            else if (!strcmp(comando, "listar"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    printf("[SERVIDOR] >> A listar todos os serviços agendados...\n");
                    sleep(2);
                    printf("\n========== SERVIÇOS ==========\n\n");
                    for (int i = 0; i < SERVICOS_ATIVOS; i++){
                        printf("|===================================================================\n");
                        printf("|> Serviço %d\n", servicos[i].id_servico);
                        printf("|\tCliente: %s | PID: %d\n|\n", servicos[i].username, servicos[i].pid_cliente);
                        printf("|\tEstado do Serviço: %s (Agendado, Em Progresso)\n", servicos[i].status_servico == 0 ? "Agendado" : servicos[i].status_servico == 1 ? "Em Progresso" : servicos[i].status_servico == 2 ? "Cancelado" : servicos[i].status_servico == 3 ? "Finalizado" : "Desconhecido");
                        printf("|\tID Veículo Designado: %d\n", servicos[i].pid_veiculo);
                        printf("|\tHora Agendada: %d\n", servicos[i].hora_agendada);
                        printf("|\tOrigem: %s\n", servicos[i].local_origem);
                        printf("|\tDistância a percorrer: %d\n", servicos[i].distancia_percurso);
                        printf("|===================================================================\n\n");
                    }
                }
            }

            // =====================================

            else if (!strcmp(comando, "utiliz"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    printf("[SERVIDOR] >> A listar todos os clientes ativos...\n");
                    sleep(2);
                    printf("\n========== CLIENTES ATIVOS ==========\n\n");
                    for (int i = 0; i < CLIENTES_ATIVOS; i++){
                        printf("\t[%d] - Cliente: %s (PID %d)\n", i, servicos[i].username, servicos[i].pid_cliente);
                    }
                }
            }

            // =====================================

            else if (!strcmp(comando, "frota"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                    /* comando_valido = 0; */
                }
                else
                {
                    printf(" ||| Comando não funcional |||\n");
                    /* strcpy(pc.comando, comando); */
                    /* comando_valido = 1; */
                    /* break; */
                }
            }

            // =====================================

            else if (!strcmp(comando, "cancelar"))
            {
                if (arg_num == 1 && strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> O argumento '<id>' deve ser numérico.\n");
                    /* comando_valido = 0; */
                }
                else if (arg_num > 2)
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' tem demasiados argumentos!\n", comando);
                    /* comando_valido = 0; */
                }
                else if (arg_num < 2)
                {
                    printf("[SERVIDOR] [ERRO] >> Falta argumento: '<id>'!\n");
                    /* comando_valido = 0; */
                }
                else
                {
                    printf(" ||| Comando não funcional |||\n");
                    /* strcpy(pc.comando, comando);
                    pc.id_servico = id; */
                    /* comando_valido = 1; */
                }
            }

            // =====================================

            else if (!strcmp(comando, "km"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    printf("|\tTotal de Kilómetros percorridos: %d\n", km_total);
                }
            }

            // =====================================

            else if (!strcmp(comando, "hora"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    printf("|\tHora Simulada do Servidor: %dh\n", hora_simulada);
                }
            }

            // =====================================

            else
            {
                printf("[SERVIDOR] [ERRO] >> Comando '%s' inválido ou desconhecido!\n", comando);
            }
        }

        printf("\n[SERVIDOR] << ");

        // =====================================

        // Fica à espera de pedidos
        // TODO: PRECISA DE REFORMULAÇÃO!!!
        /*do
        {
            n = read(fd, &pc, sizeof(PEDIDO_CLIENTE));
            if (n == -1)
            {
                printf("Erro ao ler do FIFO 'controlador'");
                return 1;
            }

            if (n == sizeof(PEDIDO_CLIENTE))
            {
                printf("Recebi %d bytes... do cliente %d\n", n, pc.pid_cliente);

                // =====================================

                // Verifica se existe espaço para o utilizador ou se ele ja existe
                /* for (posicao_pid = -1, livre = -1, i = 0; i < NCLIENTES; i++)
                {
                    // Verifica se o utilizador ja existe
                    if (clientes[i] == pc.pid_cliente)
                        posicao_pid = i;

                    // Verifica se existe espaço no array
                    // Guarda o primeiro espaço livre que encontra
                    if (livre == -1 && clientes[i] == -1)
                        livre = i;
                }

                // Se não existe não existe no array mas há uma espaço livre, adiciona ao array
                if (posicao_pid == -1 && livre != -1)
                {
                    posicao_pid = livre;
                    clientes[posicao_pid] = pc.pid_cliente;
                }
                // Se nao existe e nao tem espaço livre, avisa o utilizador e não adiciona
                if (posicao_pid == -1 && livre == -1)
                {
                    snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] Não existe espaço para o utilizador!");
                    n = write(fd, &rs, sizeof(RESPOSTA_SERVER));
                    continue;
                }

                // Se existe e o comando dado no cliente for 'terminar', remove o utilizador
                if (posicao_pid != -1 && (strcmp(pc.comando, "terminar") == 0))
                {
                    clientes[posicao_pid] = -1;
                    continue;
                } */

        // =====================================

        // No str_pid coloca o PID do utilizador formatado
        /* sprintf(str_pid, CLI_FIFO, clientes[posicao_pid]);
        // Abre o FIFO especifico do cliente
        fd_cli = open(str_pid, O_WRONLY);
        if (fd_cli < 0)
        {
            printf("[SERVIDOR] Erro ao abrir o FIFO 'cli_%d'\n", pc.pid_cliente);
            snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] Erro ao abrir o FIFO 'cli_%d'\n", pc.pid_cliente);
        }
        else
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

                // Necessário reformular por causa do array de servicos
                //printf("ID Recebido: %d\n", svc.id_servico);

                // Envia a resposta ao cliente
                snprintf(rs.msg, sizeof(rs.msg), "[SERVIDOR] CANCELAMENTO RECEBIDO");
            }
            // Será removido porque o cliente não é suposto terminar o servidor
            // Apenas está para fins de teste!
            if (strcmp(pc.comando, "terminar") == 0)
            {
                printf("Comando Recebido: %s\n", pc.comando);

                // Envia a resposta ao cliente
                printf("O cliente %s com o pid %d terminou o programa!\n", pc.username, clientes[posicao_pid]);
            }
        }
        n = write(fd_cli, &rs, sizeof(RESPOSTA_SERVER));
        printf("Comando Enviado: %s\n", rs.msg);

        // Fecha o FIFO do cliente
        close(fd_cli);
    } */

        // =====================================

        // Mostra os utilizadores ativos
        // printf("\nUTILIZADORES ATIVOS:\n");
        // for (i = 0; i < NCLIENTES; i++)
        // {
        //     printf("CLIENTES[%d] = %d\n", i, clientes[i]);
        // }
        //} while (strcmp(pc.comando, "terminar") != 0); */

        // =====================================

        pthread_join(thread_1, NULL);
        pthread_join(thread_2, NULL);
        pthread_join(thread_3, NULL);

        pthread_mutex_destroy(&mutex);

        // =====================================
    }

    terminarServidor(0);
}
