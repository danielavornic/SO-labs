// consumer.c
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/prodcon_fifo"

volatile sig_atomic_t running = 1;

void handle_signal(int sig)
{
    running = 0;
}

typedef struct {
    int prod_id;
    int value;
} Message;

int main(int argc, char* argv[])
{
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    if (argc != 2) {
        printf("Usage: %s <consumer_id>\n", argv[0]);
        return 1;
    }

    int consumer_id = atoi(argv[1]);
    srand(time(NULL) + consumer_id + 100);

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    Message msg;
    while (running) {
        if (read(fd, &msg, sizeof(Message)) == sizeof(Message)) {
            printf("Consumer %d consumed: %d (from Producer %d)\n",
                consumer_id, msg.value, msg.prod_id);
            fflush(stdout);
            usleep(200000);
        }
    }

    close(fd);
    return 0;
}