//
// Created by lelelele29 on 06/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_SHAREDMEMORYUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SHAREDMEMORYUTILS_H

int getSharedMemoryId(int id, int size);

void* getSharedMemory(int sharedMemoryId);

int getSharedMemorySize(int sharedMemoryId);

void deleteSharedMemory(int sharedMemoryId);

#endif //DISALVOFRANZA_PROGETTOSO_SHAREDMEMORYUTILS_H
