#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

static int run = 1;
static int retvalue;

void *start_routine(void *arg)
{
    int *running = arg;
    printf("child pthread init finish, arg is: %d\n", *running);
    while (*running)
    {
        printf("child pthread is running\n");
        usleep(1);
    }
    printf("child pthread exit\n");

    retvalue = 8;
    pthread_exit((void *)&retvalue);
}

int main(void)
{
    pthread_t pt;
    int ret = -1;
    int times = 3;
    int i = 0;
    int *ret_join = NULL;

    ret = pthread_create(&pt, NULL, (void *)start_routine, &run);

    if (ret != 0)
    {
        printf("create pthread fail\n");
        return 1;
    }
    usleep(1);
    for (; i < times; i++)
    {
        printf("main pthread print\n");
        usleep(1);
    }
    run = 0;
    pthread_join(pt, (void *)&ret_join);
    printf("pthread retvalue: %d\n", *ret_join);
    return 0;
}