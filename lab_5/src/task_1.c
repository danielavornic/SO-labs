#include <errno.h>
#include <linux/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SHARED_FILE "shared_data.txt"
#define MAX_BUFFER 256

// config variables
#define NUM_READERS 4
#define NUM_WRITERS 2
#define NUM_ITERATIONS 3
#define MIN_SLEEP_MS 100
#define MAX_SLEEP_MS 500

// synchronization variables
sem_t resource_access;
sem_t reader_count_mutex;
int reader_count = 0;

int total_reads = 0;
int total_writes = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

long get_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void random_sleep()
{
    usleep((MIN_SLEEP_MS + rand() % (MAX_SLEEP_MS - MIN_SLEEP_MS)) * 1000);
}

int write_to_file(const char* content)
{
    // w - to overwrite the file
    FILE* file = fopen(SHARED_FILE, "w");
    if (!file) {
        perror("Error opening file for writing");
        return -1;
    }
    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}

int read_from_file(char* buffer, size_t size)
{
    FILE* file = fopen(SHARED_FILE, "r");
    if (!file) {
        perror("Error opening file for reading");
        return -1;
    }

    if (fgets(buffer, size, file) == NULL && ferror(file)) {
        perror("Error reading from file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void* writer(void* arg)
{
    int id = *(int*)arg;
    char buffer[MAX_BUFFER];

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        random_sleep();

        // wait for resource access
        sem_wait(&resource_access);

        // write to file
        snprintf(buffer, sizeof(buffer), "Message #%d from Writer %d", i + 1, id);

        if (write_to_file(buffer) == 0) {
            printf("[Writer %d] Wrote: %s\033[0m\n", id, buffer);
        } else {
            printf("[Writer %d] Failed to write\n", id);
        }

        pthread_mutex_lock(&stats_mutex);
        total_writes++;
        pthread_mutex_unlock(&stats_mutex);

        random_sleep();
        // release resource access
        sem_post(&resource_access);

        // longer sleep between writes
        usleep(rand() % 300000 + 200000);
    }
    return NULL;
}

void* reader(void* arg)
{
    int id = *(int*)arg;
    char buffer[MAX_BUFFER];

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        random_sleep();

        sem_wait(&reader_count_mutex);
        reader_count++;
        if (reader_count == 1) {
            sem_wait(&resource_access);
        }
        sem_post(&reader_count_mutex);

        if (read_from_file(buffer, sizeof(buffer)) == 0) {
            printf("[Reader %d] Read: %s\033[0m\n", id, buffer);
        } else {
            printf("[Reader %d] Failed to read\n", id);
        }

        pthread_mutex_lock(&stats_mutex);
        total_reads++;
        pthread_mutex_unlock(&stats_mutex);

        sem_wait(&reader_count_mutex);
        reader_count--;
        if (reader_count == 0) {
            sem_post(&resource_access);
        }
        sem_post(&reader_count_mutex);

        // shorter sleep between reads
        usleep(rand() % 200000);
    }
    return NULL;
}

void initialize_file()
{
    const char* initial_message = "Initial content - File created";
    if (write_to_file(initial_message) == -1) {
        fprintf(stderr, "Failed to initialize file\n");
        exit(1);
    }
    printf("[System] File initialized with: %s\n", initial_message);
}

void cleanup()
{
    pthread_mutex_destroy(&stats_mutex);
    sem_destroy(&resource_access);
    sem_destroy(&reader_count_mutex);
}

void display_final_state()
{
    char buffer[MAX_BUFFER];
    printf("\nFinal file contents:\n");
    if (read_from_file(buffer, sizeof(buffer)) == 0) {
        printf("%s\n", buffer);
    } else {
        printf("Could not read final state\n");
    }
}

int main()
{
    srand(time(NULL));

    if (sem_init(&resource_access, 0, 1) == -1 || sem_init(&reader_count_mutex, 0, 1) == -1) {
        perror("Semaphore initialization failed");
        return 1;
    }

    initialize_file();

    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];

    // reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        if (pthread_create(&readers[i], NULL, reader, &reader_ids[i]) != 0) {
            perror("Failed to create reader thread");
            cleanup();
            return 1;
        }
    }

    // delay before creating writers
    // to ensure readers are already waiting
    usleep(100000);

    // writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        if (pthread_create(&writers[i], NULL, writer, &writer_ids[i]) != 0) {
            perror("Failed to create writer thread");
            cleanup();
            return 1;
        }
    }

    // wait for all threads to finish
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    printf("\nExecution completed!\n");
    printf("Total reads: %d\n", total_reads);
    printf("Total writes: %d\n", total_writes);
    printf("Expected reads: %d\n", NUM_READERS * NUM_ITERATIONS);
    printf("Expected writes: %d\n", NUM_WRITERS * NUM_ITERATIONS);

    display_final_state();
    cleanup();

    return 0;
}