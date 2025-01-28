#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void *producter_f(void *arg);
void *consumer_f(void *arg);

int buffer_has_item = 1;
pthread_mutex_t mutex;
pthread_cond_t cond; // condition for synchronization
int running = 1;

int main(void)
{
    pthread_t consumer_t;
    pthread_t producter_t;

    // init
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // create prodcter and consumer thread
    pthread_create(&producter_t, NULL, (void *)producter_f, NULL);
    pthread_create(&consumer_t, NULL, (void *)consumer_f, NULL);

    usleep(10000);
    running = 0;
    pthread_join(consumer_t, NULL);
    pthread_join(producter_t, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

void *producter_f(void *arg)
{
    while (running)
    {
        pthread_mutex_lock(&mutex);

        buffer_has_item++;
        printf("producting, counts: %d\n", buffer_has_item);

        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
        usleep(100);
    }
}

void *consumer_f(void *arg)
{
    while (running)
    {
        pthread_mutex_lock(&mutex);

        while (buffer_has_item == 0)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        buffer_has_item--;
        printf("consuming, counts: %d\n", buffer_has_item);
        pthread_mutex_unlock(&mutex);
        usleep(100);
    }
}
