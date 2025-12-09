// Interação com o utilizador
// Interação com a plataforma/controlador
// Agenda transportes a partir da plataforma/controlador
#include <time.h>
#include "util.h"

int main(int argc, char *argv[])
{
    // Check Inicial ao iniciar o CLiente

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

    PEDIDO_CLIENTE pc;

    // Preenche o username do utilizador
    strcpy(pc.username, argv[1]);

    // Preenche o id do utilizador deste cliente
    pc.pid = getpid();

    // Abre o FIFO do controlador
    int fd = open(SRV_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("Erro ao abrir o FIFO 'controlador'");
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
        char buffer[256];

        // Input do Utilizador
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Erro ao ler input.\n");
            continue;
        }

        // =====================================

        // Remove o \0 e \n
        buffer[strcspn(buffer, "\n")] = '\0';

        // Extrai o primeiro termo do buffer
        char *cmd = strtok(buffer, " ");

        // Verifica se o está NULL/VAZIO
        if (cmd == NULL) {
            printf("Comando vazio.\n");
            continue;
        }

        // =====================================

        else if (strcmp(cmd, "agendar") == 0) {

            char *hora = strtok(NULL, " ");
            char *origem = strtok(NULL, " ");
            char *dist = strtok(NULL, " ");
            char *extra = strtok(NULL, " ");

            if (!hora || !origem || !dist || extra != NULL) {
                printf("Uso correto: agendar <hora> <origem> <distancia>\n");
                continue;
            }

            if (!isNumber(hora)) {
                printf("[ERRO] hora deve ser numérica.\n");
                continue;
            }
            if (!isNumber(dist)) {
                printf("[ERRO] distancia deve ser numérica.\n");
                continue;
            }

            printf("Comando válido: agendar | Hora = %s | Origem = %s | Distancia = %s\n", hora, origem, dist);

            strcpy(pc.comando, cmd);
            pc.hora = atoi(hora);
            strcpy(pc.origem, origem);
            pc.distancia = atoi(dist);
            if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
                printf("[ERRO] Falha ao escrever no FIFO 'controlador'");
                return 1;
            };
            
            continue;
        }

        // =====================================
        
        else if (strcmp(cmd, "consultar") == 0) {

            char *extra = strtok(NULL, " ");
            if (extra != NULL) {
                printf("[ERRO] 'consultar' não aceita argumentos.\n");
                continue;
            }

            printf("Comando válido: consultar\n");
            strcpy(pc.comando, cmd);
            if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
                printf("[ERRO] Falha ao escrever no FIFO 'controlador'");
                return 1;
            };
            
            continue;
        }

        // =====================================

        else if (strcmp(cmd, "cancelar") == 0) {

            char *id_servico = strtok(NULL, " ");
            char *extra = strtok(NULL, " ");

            if (id_servico == NULL || extra != NULL) {
                printf("Uso correto: cancelar <id>\n");
                continue;
            }

            if (!isNumber(id_servico)) {
                printf("[ERRO] id deve ser numérico.\n");
                continue;
            }

            printf("Comando válido: cancelar id=%s\n", id_servico);
            strcpy(pc.comando, cmd);
            pc.id_servico = atoi(id_servico);            
            if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
                printf("[ERRO] Falha ao escrever no FIFO 'controlador'");
                return 1;
            };

            continue;
        }

        // =====================================

        else if (strcmp(cmd, "terminar") == 0) {

            char *extra = strtok(NULL, " ");
            if (extra != NULL) {
                printf("[ERRO] 'terminar' não aceita argumentos.\n");
                continue;
            }

            strcpy(pc.comando, cmd);
            if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
                printf("[ERRO] Falha ao escrever no FIFO 'controlador'");
                return 1;
            };
            printf("A terminar...\n");
        }

        // =====================================

        else {
            printf("Comando inválido: %s\n", cmd);
            continue;
        }

        
    } while (strcmp(pc.comando, "terminar") != 0);

    close(fd);


    // =====================================
    // LIXO QUE PODE SER USADO
    // =====================================

    // PEDIDO_CLIENTE pc;
    
    // printf("Username: %s\n", pc.username);
    // if (write(fd, &pc, sizeof(PEDIDO_CLIENTE)) == -1) {
    //     printf("[ERRO] Falha ao escrever no FIFO 'controlador'");
    //     return 1;
    // }
    // printf("Escrevi: %s\n", pc.username);

    // return 0;

    // char str[20];
    // PEDIDO_CLIENTE pc;
    // RESPOSTA_SERVER rs;

    

    // // Se sim, abre o FIFO do controlador
    // int fd = open(SRV_FIFO, O_WRONLY);
    // if (fd == -1)
    // {
    //     printf("[ERRO] Falha ao abrir o FIFO 'controlador'");
    //     exit(3);
    // }

    // printf("Registo Inicial:\n");

    // // Inicio de sessão + guarda nome na estrutura
    // strcpy(pc.username, argv[1]);
    // printf("Username: %s\n", pc.username);

    // // Cria o FIFO do cliente
    // pc.id = getpid();
    // printf("ID: %d\n", pc.id);

    // sprintf(str, CLI_FIFO, pc.id);
    // mkfifo(str, 0640);
    // int fd_cli = open(str, O_RDWR);

    // // AÇÕES DO CLIENTE
    // //  AGENDAR <HORA> <LOCAL ORIGEM> <DISTANCIA>
    // //  CONSULTAR
    // //  CANCELAR <ID>
    // //  TERMINAR - SAI DA APLICAÇÃO CLIENTE APENAS

    // //Verifica comando introduzido
    // do
    // {
    //     printf("Comandos disponiveis:\n");
    //     printf("\t- AGENDAR <HORA> <LOCAL ORIGEM> <DISTANCIA>\n");
    //     printf("\t- CONSULTAR\n");
    //     printf("\t- CANCELAR <ID>\n");
    //     printf("\t- TERMINAR\n");

    //     printf("\nDigite o comando: ");
    //     scanf("%s %d %s %d", pc.comando, &pc.hora, pc.origem, &pc.distancia);

    //     if (strcmp(pc.comando, "agendar") == 0)
    //     {
    //         write(fd, &pc, sizeof(PEDIDO_CLIENTE));
    //     }
    //     else if (strcmp(pc.comando, "consultar") == 0)
    //     {
    //         write(fd, &pc, sizeof(PEDIDO_CLIENTE));
    //         printf("%s\n", rs.msg);
    //     }
    //     else if (strcmp(pc.comando, "cancelar") == 0)
    //     {
    //         write(fd, &pc, sizeof(PEDIDO_CLIENTE));
    //     }
    //     else if (strcmp(pc.comando, "terminar") == 0)
    //     {
    //         write(fd, &pc, sizeof(PEDIDO_CLIENTE));
    //         break;
    //     }
    //     else
    //     {
    //         printf("Comando invalido!\n");
    //     }
    // } while (!(strcmp(pc.comando, "terminar") == 0));

    // // Pode consultar agendamentos que fez
    // // Pode cancelar agendamentos de um serviço previamente agendado, caso ainda não tenha sido realizado

    // // Pode fazer um ou vários agendamentos (hora, local origem, distancia de percurso)
    // // Envia pedido de agendamento para o controlador
    // // Assim que veiculo chegar, indica local de destino
    // // Não pode cancelar quando veiculo chegar

    // close(fd_cli);
    // unlink(str);
    // close(fd);
    // exit(0); // 0 = sucesso
}