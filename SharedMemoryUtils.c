#include "SharedMemoryUtils.h"
#include "KeyUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

int getSharedMemoryId(int id, int size)
{
    int sharedMemoryId = shmget(getKey(id), size, IPC_CREAT | 0644);
    if (sharedMemoryId == -1)
    {
        printf("Get shared memory id error");
    }
    return sharedMemoryId;
}

void* getSharedMemory(int sharedMemoryId)
{
    printf("1");
    void* memory = shmat(sharedMemoryId, NULL, 0);
    printf("2");
    if (memory == (void*) -1)
    {
        printf("3");
        printf("Get shared memory error");
    }
    printf("4");
    return memory;
}

int getSharedMemorySize(int sharedMemoryId)
{
    struct shmid_ds sharedMemoryInfo;
    if (shmctl(sharedMemoryId, IPC_STAT, &sharedMemoryInfo) == -1) {
        perror("shmctl (IPC_STAT)");
        return -1;
    }
    return (int) sharedMemoryInfo.shm_segsz;
}

void clearSharedMemory(int sharedMemoryId)
{
    int memorySize = getSharedMemorySize(sharedMemoryId);
    char* memory = getSharedMemory(sharedMemoryId);
    for (int i = 0; i < memorySize; i++)
    {
        memory[i] = 0;
    }
}

void deleteSharedMemory(int sharedMemoryId)
{
    if (shmctl(sharedMemoryId, IPC_RMID, NULL) == -1)
    {
        printf("Delete shared memory error");
    }
}