//
// Created by lelelele29 on 06/12/23.
//

#include "SharedMemoryUtils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>

int getSharedMemoryId(int id, unsigned long size)
{
    int sharedMemoryId = shmget(id, size, IPC_CREAT | 0644);
    if (sharedMemoryId == -1)
    {
        printf("Get shared memory id error");
    }
    return sharedMemoryId;
}

void* getSharedMemory(int sharedMemoryId)
{
    void* memory = shmat(sharedMemoryId, NULL, 0);
    if (memory == (void*) -1)
    {
        printf("Get shared memory error");
    }
    return memory;
}

void deleteSharedMemory(int sharedMemoryId)
{
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        printf("Delete shared memory error");
    }
}
