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
    for (i = 0; i < SERVICOS_ATIVOS; i++)
        if (servicos[i].status_servico == 1 || servicos[i].status_servico == 2)
            kill(servicos[i].pid_veiculo, SIGKILL);
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
        hora_simulada = hora_simulada + 1;
        sleep(1);
    }
    return NULL;
}

// serao 10 veiculos
void *veiculo(void *arg)
{

    /* pthread_mutex_lock(tdt_srv->mut);

    pthread_mutex_unlock(tdt_srv->mut); */

    //iniciar
    //informar o cliente
    //
    

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
            printf("[SERVIDOR] [COMANDO RECEBIDO]");
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
                servicos[SERVICOS_ATIVOS].id_servico = SERVICOS_ATIVOS;
                strcpy(servicos[SERVICOS_ATIVOS].username, pc.username);
                servicos[SERVICOS_ATIVOS].pid_cliente = pc.pid_cliente;
                servicos[SERVICOS_ATIVOS].status_servico = 1;
                servicos[SERVICOS_ATIVOS].hora_agendada = pc.hora;
                strcpy(servicos[SERVICOS_ATIVOS].local_origem, pc.origem);
                servicos[SERVICOS_ATIVOS].distancia_pedida = pc.distancia;
                
                char msg[50];
                sprintf(msg, "Serviço nº %d agendado!", servicos[SERVICOS_ATIVOS].id_servico);
                envia_msg_cliente(pc.pid_cliente, msg);

                SERVICOS_ATIVOS++;
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
    }

    close(fd_srv);
    return NULL;
}

/* void *cliente(void *arg)
{
    int fd_srv = open(SRV_FIFO, O_RDWR);
    PEDIDO_CLIENTE pc;

    while ((read(fd_srv, &pc, sizeof(pc)) > 0) && servidor_ativo == 1)
    {
        pthread_mutex_unlock(&mutex);
        printf("[SERVIDOR] [RECEBIDO] Comando: %s | Cliente PID: %d\n", pc.comando, pc.pid_cliente);
        pthread_mutex_unlock(&mutex);

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
                        clientes[i].pid_cliente = 0;
                        CLIENTES_ATIVOS--;
                        printf("\n[SERVIDOR] >> O cliente %s (PID %d) saiu.\n", pc.username, pc.pid_cliente);
                        printf("[SERVIDOR] << ");
                        fflush(stdout);
                        break;
                    }
                }
            }
        }
        else if (!strcmp(pc.comando, "consultar"))
        {
            sleep(1);
            printf("consultar recebido!\n");
            envia_msg_cliente(pc.pid_cliente, "Sem serviços.");
        }
    }

    if (servidor_ativo != 1)
    {
        close(fd_srv);
    }
    return NULL;
} */

// Processa comandos do administrador
void *administrador(void *arg)
{
    /* pthread_mutex_lock(tdt_srv->mut);

    pthread_mutex_unlock(tdt_srv->mut); */
    return NULL;
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
    pthread_create(&thread_1, 0, tempo, 0);
    pthread_create(&thread_2, 0, cliente, 0);
    pthread_create(&thread_3, 0, administrador, 0);

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
                    printf("[SERVIDOR] >> A listar todos os serviços agendados...\n");
                    sleep(1);
                    if (SERVICOS_ATIVOS == 0)
                    {

                        printf("[SERVIDOR] [INFO] >> Nenhum serviço agendado.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d serviços agendados.\n", SERVICOS_ATIVOS);
                        printf("\n========== SERVIÇOS ==========\n\n");
                        for (int i = 0; i < SERVICOS_ATIVOS; i++)
                        {
                            printf("|===================================================================\n");
                            printf("|> Serviço %d\n", servicos[i].id_servico);
                            printf("|- Cliente: %s | PID: %d\n|\n", servicos[i].username, servicos[i].pid_cliente);
                            printf("|- Estado do Serviço: %s \n", servicos[i].status_servico == 0 ? "Sem serviços agendados" : servicos[i].status_servico == 1 ? "Agendado"
                                                                                                                           : servicos[i].status_servico == 2   ? "Em Progresso"
                                                                                                                           : servicos[i].status_servico == 3   ? "Cancelado"
                                                                                                                                                               : "Finalizado");
                            printf("|- ID Veículo Designado: %d\n", servicos[i].pid_veiculo);
                            printf("|- Hora Agendada: %d\n", servicos[i].hora_agendada);
                            printf("|- Origem: %s\n", servicos[i].local_origem);
                            printf("|- Distância pedida: %d km\n", servicos[i].distancia_pedida);
                            printf("|===================================================================\n\n");
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
                    printf("[SERVIDOR] >> A listar todos os clientes ativos...\n");
                    sleep(1);
                    if (CLIENTES_ATIVOS <= 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Não existem clientes ativos.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d clientes ativos.\n", CLIENTES_ATIVOS);
                        printf("\n========== CLIENTES ATIVOS ==========\n\n");
                        for (int i = 0; i < CLIENTES_ATIVOS; i++)
                        {

                            printf("- Cliente: %s | PID: %d", clientes[i].username, clientes[i].pid_cliente);
                            printf(" | Estado: %s \n", servicos[i].status_servico == 1   ? "Agendado"
                                                       : servicos[i].status_servico == 2 ? "Em Progresso"
                                                       : servicos[i].status_servico == 3 ? "Cancelado"
                                                       : servicos[i].status_servico == 4 ? "Finalizado"
                                                                                         : "Sem serviços agendados");
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
                    if (SERVICOS_ATIVOS == 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Não há veículos em movimento.\n");
                    }
                    else
                    {
                        printf("[SERVIDOR] [INFO] >> Existem %d veículos em movimento.\n", SERVICOS_ATIVOS);
                        printf("\n========== VEÍCULOS EM MOVIMENTO ==========\n\n");
                        for (int i = 0; i < SERVICOS_ATIVOS; i++)
                        {
                            printf("\t- Serviço %d | Veiculo PID %d: %d%%", servicos[i].id_servico, servicos[i].pid_veiculo, servicos[i].distancia_percorrida_percentagem);
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
                    if (SERVICOS_ATIVOS == 0)
                    {
                        printf("[SERVIDOR] [INFO] >> Nenhum há serviços ativos para cancelar.\n");
                    }
                    else
                    {
                        int conta_cancelado = 0;
                        for (int i = 0; i < SERVICOS_ATIVOS; i++)
                        {
                            // Verifica se o serviço ja foi cancelado
                            if (servicos[id].id_servico == id && servicos[i].status_servico == 3)
                            {
                                printf("[SERVIDOR] [INFO] >> Serviço %d ja foi cancelado.\n", id);
                                break;
                            }
                            // Verifica se o seriço já foi finalizado
                            else if (servicos[id].id_servico == id && servicos[i].status_servico == 4)
                            {
                                printf("[SERVIDOR] [INFO] >> Serviço %d ja foi finalizado.\n", id);
                                break;
                            }
                            // Se id = 0, cancela todos os serviços (agendado ou em execução)
                            else if (id == 0 && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                            {
                                kill(servicos[i].pid_veiculo, SIGUSR1);
                                servicos[i].status_servico = 3;
                                conta_cancelado++;
                                break;
                            }
                            // Se id != 0, cancela o serviço (agendado ou em execução) especificado pedido pelo administrador
                            else if (servicos[i].id_servico == id && servicos[i].status_servico > 0 && servicos[i].status_servico < 3)
                            {
                                kill(servicos[i].pid_veiculo, SIGUSR1);
                                servicos[i].status_servico = 3;
                                conta_cancelado++;
                                printf("[SERVIDOR] [INFO] >> Serviço %d cancelado com sucesso.\n", id);
                                break;
                            }
                        }
                        // Para id = 0, apresenta uma mensagem no final de todos os serviços cancelados
                        if (conta_cancelado > 1)
                        {
                            printf("[SERVIDOR] [INFO] >> %d serviços cancelados com sucesso.\n", conta_cancelado);
                        }

                        // TODO: ENVIAR MENSAGEM AO CLIENTE AO INFORMAR QUE O SERVIÇO FOI CANCELADO PELO ADMINISTRADOR
                        envia_msg_cliente(servicos[id].pid_cliente, "Serviço cancelado pelo administrador.\n");
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
                    printf("|\tTotal de kilómetros percorridos por todos os veículos: %d km\n", km_total);
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
                    printf("|\tHora Simulada do Servidor: %d h\n", hora_simulada);
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

    // =====================================

    pthread_mutex_destroy(&mutex);
    terminarServidor(0);
}
