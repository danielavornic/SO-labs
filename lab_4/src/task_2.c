#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 5

typedef struct {
    int producer_id;
    int value;
} Message;

int pipe_fd[2];

sem_t producer_sem;
volatile sig_atomic_t running = 1;

void handle_signal(int sig)
{
    running = 0;
}

void* producer(void* arg)
{
    int id = *(int*)arg;
    srand(time(NULL) + id);

    while (running) {
        sem_wait(&producer_sem);

        Message msg;
        msg.producer_id = id;
        msg.value = rand() % 100;

        if (write(pipe_fd[1], &msg, sizeof(Message)) == sizeof(Message)) {
            printf("Producer %d produced: %d\n", id, msg.value);
            fflush(stdout);
        }

        sem_post(&producer_sem);
        usleep(100000);
    }

    return NULL;
}

void* consumer(void* arg)
{
    int id = *(int*)arg;
    srand(time(NULL) + id + 100);

    Message msg;
    while (running) {
        if (read(pipe_fd[0], &msg, sizeof(Message)) == sizeof(Message)) {
            printf("Consumer %d consumed: %d (from Producer %d)\n",
                id, msg.value, msg.producer_id);
            fflush(stdout);
            usleep(200000);
        }
    }

    return NULL;
}

int main()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Unnamed pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return 1;
    }

    sem_init(&producer_sem, 0, 3);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    while (running) {
        sleep(1);
    }

    // Cleanup threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_cancel(producers[i]);
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_cancel(consumers[i]);
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&producer_sem);
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    return 0;
}