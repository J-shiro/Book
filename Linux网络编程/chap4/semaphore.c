#include <sys/types.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>

typedef int sem_t;
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;

sem_t CreateSem(key_t key, int value)
{
    union semun sem;
    sem_t semid;
    sem.val = value;

    semid = semget(key, 1, IPC_CREAT | 0666);
    if (-1 == semid)
    {
        printf("create semaphore error\n");
        return -1;
    }

    semctl(semid, 0, SETVAL, sem);
    return semid;
}

int Sem_P(sem_t semid)
{
    struct sembuf sops = {0, +1, IPC_NOWAIT};
    return (semop(semid, &sops, 1));
}

int Sem_V(sem_t semid)
{
    struct sembuf sops = {0, -1, IPC_NOWAIT};
    return (semop(semid, &sops, 1));
}

void SetValueSem(sem_t semid, int value)
{
    union semun sem;
    sem.val = value;

    semctl(semid, 0, SETVAL, sem);
}

int GetvalueSem(sem_t semid)
{
    union semun sem;
    return semctl(semid, 0, GETVAL, sem);
}

void DestroySem(sem_t semid)
{
    union semun sem;
    sem.val = 0;
    semctl(semid, 0, IPC_RMID, sem);
}

int main(void)
{
    key_t key;
    int semid;
    int i;
    struct semid_ds buf;
    int value = 0;

    key = ftok("sem", 'a');

    semid = CreateSem(key, 100);
    for (i = 0; i <= 3; i++)
    {
        Sem_P(semid);
        Sem_V(semid);
    }
    value = GetvalueSem(semid);
    printf(" signal value is : %d\n", value);

    DestroySem(semid);
    return 0;
}
