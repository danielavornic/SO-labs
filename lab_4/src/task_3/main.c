#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/prodcon_fifo"
#define SEM_PRODUCER_NAME "/producer_sem"
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 5

pid_t producer_pids[NUM_PRODUCERS];
pid_t consumer_pids[NUM_CONSUMERS];
volatile sig_atomic_t running = 1;

void cleanup()
{
    sem_unlink(SEM_PRODUCER_NAME);
    unlink(FIFO_PATH);
}

void handle_signal(int sig)
{
    running = 0;

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        if (producer_pids[i] > 0)
            kill(producer_pids[i], SIGTERM);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        if (consumer_pids[i] > 0)
            kill(consumer_pids[i], SIGTERM);
    }
}

int main()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    cleanup();

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    sem_t* producer_sem = sem_open(SEM_PRODUCER_NAME, O_CREAT, 0666, 3);
    if (producer_sem == SEM_FAILED) {
        perror("sem_open");
        cleanup();
        return 1;
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id[2];
            sprintf(id, "%d", i);
            execl("./consumer", "consumer", id, NULL);
            exit(1);
        }
        consumer_pids[i] = pid;
    }

    usleep(100000);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id[2];
            sprintf(id, "%d", i);
            execl("./producer", "producer", id, NULL);
            exit(1);
        }
        producer_pids[i] = pid;
    }

    printf("Press Ctrl+C to stop...\n");

    while (running) {
        sleep(1);
    }

    // Wait for all processes to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        waitpid(producer_pids[i], NULL, 0);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        waitpid(consumer_pids[i], NULL, 0);
    }

    cleanup();
    return 0;
}