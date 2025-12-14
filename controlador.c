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
    servidor_ativo = 0;
    unlink(SRV_FIFO);
    int i;
    for (i = 1; i < SERVICOS_ATIVOS + 1; i++)
        if (servicos[i].status_servico == 1 || servicos[i].status_servico == 2)
            kill(servicos[i].pid_veiculo, SIGUSR1);
    for (i = 0; i < CLIENTES_ATIVOS; i++)
        if (clientes[i].pid_cliente)
            kill(clientes[i].pid_cliente, SIGUSR1);
    printf("\n| ======================================== |\n");
    printf("        [ O SERVIDOR FOI ENCERRADO ]        \n");
    printf("| ======================================== |\n");
    exit(0);
}

void *tempo(void *arg)
{
    while (servidor_ativo == 1)
    {
        sleep(1);

        pthread_mutex_lock(&mutex);
        hora_simulada = (hora_simulada + 1) % 24;

        pthread_cond_broadcast(&cond_hora);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// serao 10 veiculos
void *veiculo(void *arg)
{

    int i = 0;

    pthread_mutex_lock(&mutex);

    while (servidor_ativo)
    {
        while (servidor_ativo &&
               !(servicos[i].status_servico == 1 &&
                 servicos[i].hora_agendada == hora_simulada))
        {
            pthread_cond_wait(&cond_hora, &mutex);
        }

        if (!servidor_ativo)
            break;

        char dist[16], idx[16];
        snprintf(dist, sizeof(dist), "%d", servicos[i].distancia_pedida);
        snprintf(idx, sizeof(idx), "%d", i);

        pthread_mutex_unlock(&mutex);

        execl("./veiculo", "veiculo",
              dist, servicos[i].local_origem, idx, NULL);

        pthread_mutex_lock(&mutex);
        i = (i + 1) % NSERVICOS;
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

// Processo comandos do cliente
void *cliente(void *arg)
{
    int fd_srv = open(SRV_FIFO, O_RDWR);
    if (fd_srv == -1)
    {
        printf("[SERVIDOR] Erro ao abrir o FIFO do servidor");
        return NULL;
    }

    PEDIDO_CLIENTE pc;
    while (servidor_ativo)
    {
        ssize_t n = read(fd_srv, &pc, sizeof(pc));
        if (n > 0)
        {

            pthread_mutex_lock(&mutex);
            printf("\n[SERVIDOR] << [COMANDO RECEBIDO: %s] << [CLIENTE]\n", pc.comando);
            pthread_mutex_unlock(&mutex);

            // Processar o comando recebido
            if (!strcmp(pc.comando, "novo_cliente"))
            {
                if (CLIENTES_ATIVOS < NCLIENTES)
                {
                    strcpy(clientes[CLIENTES_ATIVOS].username, pc.username);
                    clientes[CLIENTES_ATIVOS].pid_cliente = pc.pid_cliente;

                    printf("\n[SERVIDOR] >> O cliente %s (PID %d) entrou.\n", clientes[CLIENTES_ATIVOS].username, clientes[CLIENTES_ATIVOS].pid_cliente);
                    printf("[SERVIDOR] << ");
                    CLIENTES_ATIVOS++;
                    fflush(stdout);
                    envia_msg_cliente(pc.pid_cliente, "Conectado com o servidor!");
                }
                else
                {
                    envia_msg_cliente(pc.pid_cliente, "O Servidor está cheio!");
                    kill(pc.pid_cliente, SIGINT);
                }
            }
            else if (!strcmp(pc.comando, "terminar"))
            {
                if (CLIENTES_ATIVOS > 0)
                {
                    int i;
                    for (i = 0; i < CLIENTES_ATIVOS; i++)
                    {
                        if (clientes[i].pid_cliente == pc.pid_cliente)
                        {
                            clientes[i].pid_cliente = -1;
                            strcpy(clientes[i].username, "Utilizador Excluido");
                            CLIENTES_ATIVOS--;
                            printf("\n[SERVIDOR] >> O cliente %s (PID %d) saiu.\n", pc.username, pc.pid_cliente);
                            printf("[SERVIDOR] << ");
                            fflush(stdout);
                            break;
                        }
                    }
                }
            }
            else if (!strcmp(pc.comando, "agendar"))
            {
                if (SERVICOS_ATIVOS < NSERVICOS)
                {
                    servicos[SERVICOS_ATIVOS].id_servico = ++SERVICOS_ATIVOS;
                    strcpy(servicos[SERVICOS_ATIVOS].username, pc.username);
                    servicos[SERVICOS_ATIVOS].pid_cliente = pc.pid_cliente;
                    servicos[SERVICOS_ATIVOS].status_servico = 1;
                    servicos[SERVICOS_ATIVOS].hora_agendada = pc.hora;
                    strcpy(servicos[SERVICOS_ATIVOS].local_origem, pc.origem);
                    servicos[SERVICOS_ATIVOS].distancia_pedida = pc.distancia;

                    char msg[50];
                    sprintf(msg, "Serviço nº %d agendado!", servicos[SERVICOS_ATIVOS].id_servico);
                    envia_msg_cliente(pc.pid_cliente, msg);
                }
                else
                {
                    envia_msg_cliente(pc.pid_cliente, "O Servidor está cheio e não pode lidar com mais serviços! Por favor, reinicie!");
                }
            }
            else if (!strcmp(pc.comando, "consultar"))
            {
                int encontrou_servicos = 0; // Flag para verificar se há serviços
                for (int i = 1; i < SERVICOS_ATIVOS + 1; i++)
                {
                    usleep(100);
                    if (servicos[i].pid_cliente == pc.pid_cliente)
                    {
                        encontrou_servicos = 1;
                        char msg[1024];
                        sprintf(msg, "[Serviço %d] >> Origem: %s | Distância: %d km | Hora: %d | Estado: %s",
                                servicos[i].id_servico,
                                servicos[i].local_origem,
                                servicos[i].distancia_pedida,
                                servicos[i].hora_agendada,
                                servicos[i].status_servico == 0 ? "Sem serviços agendados" : servicos[i].status_servico == 1 ? "Agendado"
                                                                                                                            : servicos[i].status_servico == 2   ? "Em Progresso"
                                                                                                                            : servicos[i].status_servico == 3   ? "Cancelado"
                                                                                                                             : servicos[i].status_servico == 4   ? "Finalizado"
                                                                                                                             : servicos[i].status_servico == 5   ? "Cancelado"
                                                                                                                                                                : "Desconhecido");
                        envia_msg_cliente(pc.pid_cliente, msg);
                    }
                }

                if (!encontrou_servicos)
                {
                    envia_msg_cliente(pc.pid_cliente, "Não há serviços agendados!");
                }
            }
            else if (!strcmp(pc.comando, "cancelar"))
            {
                if (SERVICOS_ATIVOS == 0)
                {
                    printf("[SERVIDOR] [INFO] >> Não há serviços ativos para cancelar.\n");
                }
                else
                {
                    int conta_cancelado = 0;
                    for (int i = 1; i < SERVICOS_ATIVOS + 1; i++)
                    {
                        // Verifica se o serviço ja foi cancelado
                        if (servicos[i].id_servico == pc.id_servico && servicos[i].status_servico == 3)
                        {
                            printf("[SERVIDOR] [INFO] >> Serviço %d ja foi cancelado.\n", pc.id_servico);
                        }
                        // Verifica se o seriço já foi finalizado
                        else if (servicos[i].id_servico == pc.id_servico && servicos[i].status_servico == 4)
                        {
                            printf("[SERVIDOR] [INFO] >> Serviço %d ja foi finalizado.\n", pc.id_servico);
                        }
                        // Se id = 0, cancela todos os serviços (agendado ou em execução)
                        else if (pc.id_servico == 0 && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                        {
                            servicos[i].status_servico = 3;
                            conta_cancelado++;
                        }
                        // Se id != 0, cancela o serviço (agendado ou em execução) especificado pedido pelo administrador
                        else if (servicos[i].id_servico == pc.id_servico && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                        {
                            servicos[i].status_servico = 3;
                            conta_cancelado++;
                            printf("[SERVIDOR] [INFO] >> Serviço %d cancelado com sucesso.\n", pc.id_servico);
                        }
                    }

                    char mensagem[512];
                    // Para id = 0, apresenta uma mensagem no final de todos os serviços cancelados
                    if (conta_cancelado == 1)
                    {
                        for (int j = 1; j < SERVICOS_ATIVOS + 1; j++)
                        {
                            if ((pc.id_servico == 0 && servicos[j].status_servico == 3) ||
                                (pc.id_servico != 0 && servicos[j].id_servico == pc.id_servico && servicos[j].status_servico == 3))
                            {
                                sprintf(mensagem, "Serviço %d cancelado pelo cliente.", pc.id_servico == 0 ? servicos[j].id_servico : pc.id_servico);
                                envia_msg_cliente(servicos[j].pid_cliente, mensagem);
                                servicos[j].status_servico = 5;
                            }
                        }
                    }
                    else if (conta_cancelado > 1)
                    {
                        printf("[SERVIDOR] [INFO] >> %d serviços cancelados com sucesso.\n", conta_cancelado);
                        for (int j = 1; j < SERVICOS_ATIVOS + 1; j++)
                        {
                            if ((pc.id_servico == 0 && servicos[j].status_servico == 3) ||
                                (pc.id_servico != 0 && servicos[j].id_servico == pc.id_servico && servicos[j].status_servico == 3))
                            {
                                sprintf(mensagem, "Serviço %d cancelado pelo cliente.", pc.id_servico == 0 ? servicos[j].id_servico : pc.id_servico);
                                envia_msg_cliente(servicos[j].pid_cliente, mensagem);
                                servicos[j].status_servico = 5;
                            }
                        }
                    }
                }
            }

            else
            {
                printf("[SERVIDOR] >> Comando desconhecido: %s\n", pc.comando);
            }
        }
        else
        {
            perror("[SERVIDOR] Erro ao ler do FIFO");
        }

        printf("\n[SERVIDOR] << ");
        fflush(stdout);
    }

    close(fd_srv);
    return NULL;
}

void mostraComandosServidor()
{
    printf("_______________________________________________________________________________________\n\n");
    printf("[ COMANDOS DISPONIVEIS ]    [ DESCRIÇÕES ]\n\n");
    printf("> ajuda                     » Mostra este painel de comandos\n");
    printf("> listar                    » Mostra todos os servicos agendados\n");
    printf("> utiliz                    » Mostra utilizadores conectados\n");
    printf("> frota                     » Mostra percentagem de viagem dos veiculos\n");
    printf("> cancelar <id_servico>     » Cancela um servico pelo seu id (0 = todos)\n");
    printf("> km                        » Mostra total de km percorridos por todos os veiculos\n");
    printf("> hora                      » Mostra a hora simulada atual do servidor\n");
    printf("> terminar                  » Termina o Servidor e todos os Clientes ligados\n");
    printf("_______________________________________________________________________________________\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, terminarServidor);
    signal(SIGUSR1, terminarServidor);
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

    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, 0, tempo, 0);
    pthread_create(&thread_2, 0, cliente, 0);

    // =====================================

    // Apresentação ao administrador
    printf("\n| ============================================ |\n");
    printf("            [ O SERVIDOR FOI INICIADO ]         \n");
    printf("| ============================================ |\n");

    printf("\n- Bem-vindo, Administrador (PID %d)\n", pc.pid_cliente);
    printf("- Dica: use o comando 'ajuda'\n\n");

    printf("[SERVIDOR] << ");

    // =====================================

    // Input e Validação do Administraador
    char buffer[128], comando[20], extra[20];
    int id = 0 /* , distancia = 0, comando_valido = 0 */;
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        pthread_mutex_lock(&mutex);
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
            {
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
            }

            else if (!strcmp(comando, "estr_pc"))
            {
                printf("pid_cliente: %d\n", pc.pid_cliente);
                printf("username: %s\n", pc.username);
                printf("comando: %s\n", pc.comando);
                printf("hora: %d\n", pc.hora);
                printf("id_servico: %d\n", pc.id_servico);
                printf("origem: %s\n", pc.origem);
                printf("distancia: %d\n", pc.distancia);
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
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A listar todos os serviços agendados...\n");
                    sleep(1);
                    if (SERVICOS_ATIVOS == 0)
                    {

                        printf("[SERVIDOR] [INFO] >> Nenhum serviço agendado.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d serviços agendados.\n", SERVICOS_ATIVOS);
                        printf("\n========== SERVIÇOS ==========\n\n");
                        for (int i = 1; i < SERVICOS_ATIVOS + 1; i++)
                        {
                            printf("========================================\n");
                            printf("> Serviço %d\n", servicos[i].id_servico);
                            printf("\t- Cliente: %s | PID: %d\n", servicos[i].username, servicos[i].pid_cliente);
                            printf("\t- Estado do Serviço: %s \n", servicos[i].status_servico == 0 ? "Sem serviços agendados" : servicos[i].status_servico == 1 ? "Agendado"
                                                                                                                            : servicos[i].status_servico == 2   ? "Em Progresso"
                                                                                                                            : servicos[i].status_servico == 3   ? "Cancelado"
                                                                                                                             : servicos[i].status_servico == 4   ? "Finalizado"
                                                                                                                             : servicos[i].status_servico == 5   ? "Cancelado"
                                                                                                                                                                : "Desconhecido");
                            printf("\t- ID Veículo Designado: %d\n", servicos[i].pid_veiculo);
                            printf("\t- Hora Agendada: %d\n", servicos[i].hora_agendada);
                            printf("\t- Origem: %s\n", servicos[i].local_origem);
                            printf("\t- Distância pedida: %d km\n", servicos[i].distancia_pedida);
                            printf("========================================\n\n");
                        }
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
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A listar todos os clientes ativos...\n");
                    sleep(1);
                    if (CLIENTES_ATIVOS <= 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Não existem clientes ativos.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d clientes ativos.\n", CLIENTES_ATIVOS);
                        printf("\n========== CLIENTES ATIVOS ==========\n\n");
                        for (int i = 0, j = i + 1; i < CLIENTES_ATIVOS; i++)
                        {

                            printf("- Cliente: %s | PID: %d", clientes[i].username, clientes[i].pid_cliente);
                            printf(" | Estado: %s \n", servicos[i].status_servico == 0 ? "Sem serviços agendados" : servicos[i].status_servico == 1 ? "Agendado"
                                                                                                                            : servicos[i].status_servico == 2   ? "Em Progresso"
                                                                                                                            : servicos[i].status_servico == 3   ? "Cancelado"
                                                                                                                             : servicos[i].status_servico == 4   ? "Finalizado"
                                                                                                                             : servicos[i].status_servico == 5   ? "Cancelado"
                                                                                                                                                                : "Desconhecido");
                        }
                    }
                }
            }

            // =====================================

            else if (!strcmp(comando, "frota"))
            {

                if (arg_num > 1 || strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' não aceita argumentos.\n", comando);
                }
                else
                {
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A listar todos os VEÍCULOS em movimento...\n");
                    sleep(1);
                    if (SERVICOS_ATIVOS == 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Não há veículos em movimento.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d veículos em movimento.\n", SERVICOS_ATIVOS);
                        printf("\n========== VEÍCULOS EM MOVIMENTO ==========\n\n");
                        for (int i = 1; i < SERVICOS_ATIVOS + 1; i++)
                        {
                            printf("- Serviço %d | Veiculo PID %d: %d%%\n", servicos[i].id_servico, servicos[i].pid_veiculo, servicos[i].distancia_percorrida_percentagem);
                        }
                    }
                }
            }

            // =====================================

            else if (!strcmp(comando, "cancelar"))
            {
                if (arg_num == 1 && strcmp(comando, buffer))
                {
                    printf("[SERVIDOR] [ERRO] >> O argumento '<id>' deve ser numérico.\n");
                }
                else if (arg_num > 2)
                {
                    printf("[SERVIDOR] [ERRO] >> '%s' tem demasiados argumentos!\n", comando);
                }
                else if (arg_num < 2)
                {
                    printf("[SERVIDOR] [ERRO] >> Falta argumento: '<id>'!\n");
                }
                else if (id < 0)
                {
                    printf("[SERVIDOR] [ERRO] >> O argumento '<id>' deve ser um valor positivo.\n");
                }
                else
                {
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A preparar cancelamentos de serviços...\n");
                    sleep(1);
                    if (SERVICOS_ATIVOS == 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Não há serviços ativos para cancelar.\n");
                    }
                    else
                    {
                        int conta_cancelado = 0;
                        int id_unico_encontrado = 0;
                        for (int i = 1; i < SERVICOS_ATIVOS + 1; i++)
                        {
                            // Verifica se o serviço ja foi cancelado
                            if (servicos[i].id_servico == id && servicos[i].status_servico == 3)
                            {
                                printf("[SERVIDOR] [INFO] >> Serviço %d ja foi cancelado.\n", id);
                            }
                            // Verifica se o seriço já foi finalizado
                            else if (servicos[i].id_servico == id && servicos[i].status_servico == 4)
                            {
                                printf("[SERVIDOR] [INFO] >> Serviço %d ja foi finalizado.\n", id);
                            }
                            // Se id = 0, cancela todos os serviços (agendado ou em execução)
                            else if (id == 0 && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                            {
                                servicos[i].status_servico = 3;
                                conta_cancelado++;
                            }
                            // Se id != 0, cancela o serviço (agendado ou em execução) especificado pedido pelo administrador
                            else if (servicos[i].id_servico == id && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                            {
                                servicos[i].status_servico = 3;
                                conta_cancelado++;
                                id_unico_encontrado = i;
                                printf("[SERVIDOR] [INFO] >> Serviço %d cancelado com sucesso.\n", id);
                            }
                        }

                        char mensagem[512];
                        // Para id = 0, apresenta uma mensagem no final de todos os serviços cancelados
                        if (conta_cancelado == 1)
                        {
                            for (int j = 1; j < SERVICOS_ATIVOS + 1; j++)
                            {
                                if ((id == 0 && servicos[j].status_servico == 3) ||
                                    (id != 0 && servicos[j].id_servico == id && servicos[j].status_servico == 3))
                                {
                                    sprintf(mensagem, "Serviço %d cancelado pelo administrador.", id == 0 ? servicos[j].id_servico : id);
                                    envia_msg_cliente(servicos[j].pid_cliente, mensagem);
                                    servicos[j].status_servico = 5;
                                }
                            }
                        }
                        else if (conta_cancelado > 1)
                        {
                            printf("[SERVIDOR] [INFO] >> %d serviços cancelados com sucesso.\n", conta_cancelado);
                            for (int j = 1; j < SERVICOS_ATIVOS + 1; j++)
                            {
                                if ((id == 0 && servicos[j].status_servico == 3) ||
                                    (id != 0 && servicos[j].id_servico == id && servicos[j].status_servico == 3))
                                {
                                    sprintf(mensagem, "Serviço %d cancelado pelo administrador.", id == 0 ? servicos[j].id_servico : id);
                                    envia_msg_cliente(servicos[j].pid_cliente, mensagem);
                                    servicos[j].status_servico = 5;
                                }
                            }
                        }
                    }
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
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A calcular distância total percorrida por todos os veículos...\n");
                    sleep(1);
                    printf("- Total de kilómetros percorridos por todos os veículos: %d km\n", km_total);
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
                    printf("[SERVIDOR] [ESPERA SIMULADA] >> A obter hora do servidor...\n");
                    sleep(1);
                    printf("- Hora Simulada do Servidor: %d h\n", hora_simulada);
                }
            }

            // =====================================

            else
            {
                printf("[SERVIDOR] [ERRO] >> Comando '%s' inválido ou desconhecido!\n", comando);
            }
        }

        printf("\n[SERVIDOR] << ");
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_destroy(&mutex);
    terminarServidor(0);
}