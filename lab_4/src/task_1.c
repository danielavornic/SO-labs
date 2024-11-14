#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void sigusr1_handler(int signum)
{
    printf("SIGUSR1 received!\n");
}

void sigusr2_handler(int signum)
{
    printf("SIGUSR2 received! Printing random characters and terminating...\n");

    srand(time(NULL));

    for (int i = 0; i < 100; i++) {
        printf("%c", (rand() % (126 - 33 + 1)) + 33);
    }
    printf("\n");

    exit(0);
}

int main()
{
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    printf("PID: %d\n", getpid());

    while (1) {
        sleep(1);
    }

    return 0;
}