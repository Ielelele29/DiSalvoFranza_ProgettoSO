//
// Created by lelelele29 on 04/12/23.
//

#include "SemaphoreUtils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>

int getSemaphore(pid_t pid)
{
    int semId = semget(pid, 1, IPC_CREAT | 0644);
    if (semId == -1)
    {
        printf("ERRORE");
        return -1;
    }
    return semId;
}

void deleteSemaphore(int semId)
{
    if (semId != -1)
    {
        if (semctl(semId, 0, IPC_RMID) == -1)
        {
            printf("ERRORE");
        }
    }
}

int waitAndLockSemaphore(int semId)
{
    struct sembuf semOperation;
    semOperation.sem_num = 0;
    semOperation.sem_op = -1;
    semOperation.sem_flg = 0;
    return semop(semId, &semOperation, 1);
}

int unlockSemaphore(int semId)
{
    struct sembuf semOperation;
    semOperation.sem_num = 0;
    semOperation.sem_op = 1;
    semOperation.sem_flg = 0;
    return semop(semId, &semOperation, 1);
}
