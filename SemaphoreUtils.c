#include "SemaphoreUtils.h"
#include "KeyUtils.h"
#include <stdio.h>
#include <sys/sem.h>

int getSemaphore(int id)
{
    int semId = semget(getKey(id), 1, IPC_CREAT | 0644);
    if (semId == -1)
    {
        perror("Get semaphore error");
        return -1;
    }
    return semId;
}

void deleteSemaphore(int semaphoreId)
{
    if (semaphoreId != -1)
    {
        if (semctl(semaphoreId, 0, IPC_RMID) == -1)
        {
            perror("Delete semaphore error");
        }
    }
}

int waitSemaphore(int semaphoreId)
{
    if (semaphoreId != -1)
    {
        struct sembuf semOperation;
        semOperation.sem_num = 0;
        semOperation.sem_op = 0;
        semOperation.sem_flg = 0;
        int sem = semop(semaphoreId, &semOperation, 1);
        if (sem == -1)
        {
            perror("Wait semaphore error");
        }
        return sem;
    }
    return -1;
}

int waitAndLockSemaphore(int semaphoreId)
{
    if (semaphoreId != -1)
    {
        struct sembuf semOperation;
        semOperation.sem_num = 0;
        semOperation.sem_op = -1;
        semOperation.sem_flg = 0;
        int sem = semop(semaphoreId, &semOperation, 1);
        if (sem == -1)
        {
            perror("Wait and lock semaphore error");
        }
        return sem;
    }
    return -1;
}

int unlockSemaphore(int semaphoreId)
{
    if (semaphoreId != -1)
    {
        struct sembuf semOperation;
        semOperation.sem_num = 0;
        semOperation.sem_op = 1;
        semOperation.sem_flg = 0;
        int sem = semop(semaphoreId, &semOperation, 1);
        if (sem == -1)
        {
            perror("Unlock semaphore error");
        }
        return sem;
    }
    return -1;
}
