#ifndef DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H

#include "CustomTypes.h"

int getSemaphore(int id);

void deleteSemaphore(int semId);

int waitSemaphore(int semId);

int waitAndLockSemaphore(int semId);

int unlockSemaphore(int semId);

boolean isLockedByThisProcess(int semaphoreId);

#endif //DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
