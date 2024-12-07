#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// configuration
#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define NUM_ITEMS_PER_PRODUCER 4
#define MAX_ITEM_VALUE 100

typedef struct {
    int items[BUFFER_SIZE];
    int in; // index for next insert
    int out; // index for next remove
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} BoundedBuffer;

BoundedBuffer buffer;

int total_items_produced = 0;
int total_items_consumed = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void buffer_init(BoundedBuffer* buffer)
{
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
}

void buffer_insert(BoundedBuffer* buffer, int item, int producer_id)
{
    pthread_mutex_lock(&buffer->mutex);

    // if buffer is full, wait
    while (buffer->count == BUFFER_SIZE) {
        printf("[Producer %d] Buffer full", producer_id);
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    buffer->items[buffer->in] = item;
    buffer->in = (buffer->in + 1) % BUFFER_SIZE;
    buffer->count++;

    printf("[Producer %d] Produced: %d (Buffer count: %d)\033[0m\n",
        producer_id, item, buffer->count);

    // signal that buffer is not empty
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

int buffer_remove(BoundedBuffer* buffer, int consumer_id)
{
    pthread_mutex_lock(&buffer->mutex);

    // wait while buffer is empty
    while (buffer->count == 0) {
        printf("[Consumer %d] Buffer empty", consumer_id);
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    int item = buffer->items[buffer->out];
    buffer->out = (buffer->out + 1) % BUFFER_SIZE;
    buffer->count--;

    printf("[Consumer %d] Consumed: %d (Buffer count: %d)\033[0m\n",
        consumer_id, item, buffer->count);

    // signal that buffer is not full
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    return item;
}

void* producer(void* arg)
{
    int id = *(int*)arg;

    for (int i = 0; i < NUM_ITEMS_PER_PRODUCER; i++) {
        int item = rand() % MAX_ITEM_VALUE + 1;
        buffer_insert(&buffer, item, id);

        pthread_mutex_lock(&stats_mutex);
        total_items_produced++;
        pthread_mutex_unlock(&stats_mutex);

        usleep(rand() % 500000);
    }

    return NULL;
}

void* consumer(void* arg)
{
    int id = *(int*)arg;
    int items_to_consume = (NUM_PRODUCERS * NUM_ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        int item = buffer_remove(&buffer, id);

        pthread_mutex_lock(&stats_mutex);
        total_items_consumed++;
        pthread_mutex_unlock(&stats_mutex);

        usleep(rand() % 800000);
    }

    return NULL;
}

void display_buffer_status()
{
    printf("\nBuffer status:\n");
    printf("Items in buffer: %d\n", buffer.count);
    printf("Next insert position: %d\n", buffer.in);
    printf("Next remove position: %d\n", buffer.out);
    printf("Current buffer contents: ");

    if (buffer.count == 0) {
        printf("(empty)");
    } else {
        int pos = buffer.out;
        for (int i = 0; i < buffer.count; i++) {
            printf("%d ", buffer.items[pos]);
            pos = (pos + 1) % BUFFER_SIZE;
        }
    }
    printf("\n");
}

int main()
{
    // seed random number generator
    srand(time(NULL));

    buffer_init(&buffer);

    // thread IDs
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer, &producer_ids[i]) != 0) {
            perror("Failed to create producer thread");
            return 1;
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]) != 0) {
            perror("Failed to create consumer thread");
            return 1;
        }
    }

    // wait for threads to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("\nExecution completed!\n");
    printf("Total items produced: %d\n", total_items_produced);
    printf("Total items consumed: %d\n", total_items_consumed);
    printf("Expected items: %d\n", NUM_PRODUCERS * NUM_ITEMS_PER_PRODUCER);

    display_buffer_status();

    pthread_mutex_destroy(&buffer.mutex);
    pthread_cond_destroy(&buffer.not_full);
    pthread_cond_destroy(&buffer.not_empty);
    pthread_mutex_destroy(&stats_mutex);

    return 0;
}