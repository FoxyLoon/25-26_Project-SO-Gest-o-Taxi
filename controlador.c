#include "util.h"

int main(int argc, char *argv[]) {
    int fd;
    mkfifo(SRV_FIFO, 0640);
    fd = open(SRV_FIFO, O_RDWR);

    return 0;
}