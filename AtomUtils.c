//
// Created by lelelele29 on 06/12/23.
//

#include "AtomUtils.h"
#include "SharedMemoryUtils.h"
#include "SemaphoreUtils.h"

int createAtomMemory(int maxAtoms)
{
    int atomMemory = getSharedMemoryId(10000, sizeof(int) * maxAtoms);
    int* atoms = getSharedMemory(atomMemory);
    for (int i = 0; i < maxAtoms; i++)
    {
        atoms[i] = -1;
    }
    return atomMemory;
}

int getAtomMemoryId(int maxAtoms)
{
    return getSharedMemoryId(10000, sizeof(int) * maxAtoms);
}

int* getAtomMemory(int atomMemoryId)
{
    return getSharedMemory(atomMemoryId);
}

int getFirstFreeAtomSlot(int atomMemoryId, int maxAtoms)
{
    int* atoms = getSharedMemory(atomMemoryId);
    for (int i = 0; i < maxAtoms; i++)
    {
        if (atoms[i] == -1)
        {
            return i;
        }
    }
    return -1;
}

int getAtomSemaphore()
{
    return getSemaphore(10000);
}
