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
#define SEM_PRODUCER_NAME "/producer_sem"

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
        printf("Usage: %s <prod_id>\n", argv[0]);
        return 1;
    }

    int prod_id = atoi(argv[1]);
    srand(time(NULL) + prod_id);

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    sem_t* producer_sem = sem_open(SEM_PRODUCER_NAME, 0);
    if (producer_sem == SEM_FAILED) {
        perror("sem_open");
        close(fd);
        return 1;
    }

    while (running) {
        sem_wait(producer_sem);

        Message msg;
        msg.prod_id = prod_id;
        msg.value = rand() % 100;

        if (write(fd, &msg, sizeof(Message)) == sizeof(Message)) {
            printf("Producer %d produced: %d\n", prod_id, msg.value);
            fflush(stdout);
        }

        sem_post(producer_sem);
        usleep(100000);
    }

    close(fd);
    sem_close(producer_sem);
    return 0;
}