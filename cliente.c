// Interação com o utilizador
// Interação com a plataforma/controlador
// Agenda transportes a partir da plataforma/controlador

// Pode consultar agendamentos que fez
// Pode cancelar agendamentos de um serviço previamente agendado, caso ainda não tenha sido realizado

// Pode fazer um ou vários agendamentos (hora, local origem, distancia de percurso)
// Envia pedido de agendamento para o controlador
// Assim que veiculo chegar, indica local de destino
// Não pode cancelar quando veiculo chegar

#include <time.h>
#include "util.h"

int main(int argc, char *argv[])
{
    // Check Inicial ao iniciar o CLiente
    setbuf(stdout, NULL);

    if (argc < 2)
    {
        printf("[Erro ao Iniciar] Uso correto: ./cliente <username>\n");
        return 1;
    }

    // Verifica se o controlador está a funcionar
    if (access(SRV_FIFO, F_OK) != 0)
    {
        printf("[ERRO] O 'controlador' não está a funcionar!\n");
        exit(3);
    };

    // =====================================

    // Definicoes de estruturas
    PEDIDO_CLIENTE pc;
    RESPOSTA_SERVER rs;

    // =====================================

    int n, fd, fd_cli;

    // Usado para verificar quando deve ler ou escrever
    fd_set fds;
    int res;

    // Usado para gerir pids dos clientes no FIFOs
    char str_pid[20];

    // =====================================

    // Preenche o username do utilizador
    strcpy(pc.username, argv[1]);

    // Abre o FIFO do controlador
    fd = open(SRV_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("Erro ao abrir o FIFO 'controlador'");
        return 1;
    }

    // Cria o FIFO do cliente (Exemplo: cli_1234)
    pc.pid = getpid();
    sprintf(str_pid, CLI_FIFO, pc.pid);
    mkfifo(str_pid, 0640);
    fd_cli = open(str_pid, O_RDWR | O_NONBLOCK);
    if (fd_cli == -1)
    {
        printf("Erro ao abrir o FIFO 'cli_%d'", pc.pid);
        return 1;
    }

    // =====================================

    printf("Bem-vindo, %s!\n", pc.username);
    printf("O teu pid de cliente é %d\n", pc.pid);

    printf("\nComandos disponíveis:\n");
    printf("\t- agendar <hora> <origem> <distancia>\n");
    printf("\t- consultar\n");
    printf("\t- cancelar <id>\n");
    printf("\t- terminar\n");

    // =====================================

    // Validação de Input
    do
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

                char buffer[128];

                // Input do Utilizador
                if (!fgets(buffer, sizeof(buffer), stdin))
                {
                    printf("Erro ao ler input.\n");
                    continue;
                }

                // =====================================

                buffer[strcspn(buffer, "\n")] = '\0';

                // Extrai o primeiro termo do buffer
                char *cmd = strtok(buffer, " ");

                // Verifica se o está NULL/VAZIO
                if (cmd == NULL)
                {
                    printf("Comando vazio.\n");
                    continue;
                }

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
                    pc.id_servico = atoi(id_servico);
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
    unlink(str_pid);
    close(fd);

    exit(0);
}
