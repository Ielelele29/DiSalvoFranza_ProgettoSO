#ifndef DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H

int getSemaphore(int id);

void deleteSemaphore(int semId);

int waitSemaphore(int semId);

int waitAndLockSemaphore(int semId);

int unlockSemaphore(int semId);


#endif //DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
