// Simula funcionamente do veiculo automatico

#include "util.h"

int main(int argc, char *argv[])
{
    // ./veiculo nome distancia origem indice
    int dist = atoi(argv[1]), indice = atoi(argv[4]);
    char *orig = argv[3];
    int pid = (int)getpid();

    int dist_fata = dist;
    int checkpoint = -1;

    setbuf(stdout, NULL);

    // Avisa cliente
    char f[50];
    sprintf(f, CLI_FIFO, pid);
    int fd = open(f, O_WRONLY);
    if (fd != -1)
    {
        RESPOSTA_SERVER r;
        sprintf(r.msg, "Veiculo chegou a %s. Toca a andar...", orig);
        write(fd, &r, sizeof(r));
        close(fd);
    }

    fd = open(f, O_WRONLY);

    if (fd != -1)
    {

        while (dist_fata > 0)
        {
            sleep(1);
            dist_fata--;
            int dist_percorida = dist - dist_fata;
            int perc = (dist_percorida * 100) / dist;

            if (perc / 10 > checkpoint / 10)
            {
                checkpoint = (perc / 10) * 10;

                servicos[indice].distancia_percorrida_percentagem = checkpoint;
                printf("%d%%", servicos[indice].distancia_percorrida_percentagem);
                fflush(stdout);

                RESPOSTA_SERVER r;
                sprintf(r.msg, "%d%%", servicos[indice].distancia_percorrida_percentagem);
                write(fd, &r, sizeof(r));
            }
        }

        close(fd);
    }

    // Fim
    fd = open(f, O_WRONLY);
    if (fd != -1)
    {
        RESPOSTA_SERVER r;
        strcpy(r.msg, "Chegou ao destino.");
        write(fd, &r, sizeof(r));
        close(fd);
    }

    return 0;
}