#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

void *producter_f(void *arg);
void *consumer_f(void *arg);

sem_t sem;
int running = 1;

int main(void)
{
    pthread_t consumer_t;
    pthread_t producter_t;

    // init
    sem_init(&sem, 0, 16);

    // create prodcter and consumer thread
    pthread_create(&producter_t, NULL, (void *)producter_f, NULL);
    pthread_create(&consumer_t, NULL, (void *)consumer_f, NULL);

    sleep(1);
    running = 0;
    pthread_join(consumer_t, NULL);
    pthread_join(producter_t, NULL);

    sem_destroy(&sem);

    return 0;
}

void *producter_f(void *arg)
{
    int semval = 0;
    while (running)
    {
        usleep(1);

        sem_post(&sem);
        sem_getvalue(&sem, &semval);
        printf("producting, counts: %d\n", semval);
    }
}

void *consumer_f(void *arg)
{
    int semval = 0;
    while (running)
    {
        usleep(1);
        sem_wait(&sem);
        sem_getvalue(&sem, &semval);
        printf("consuming, counts: %d\n", semval);
    }
}
