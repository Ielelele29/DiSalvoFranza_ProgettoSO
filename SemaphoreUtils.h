//
// Created by lelelele29 on 04/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H

#include <sys/types.h>

int getSemaphore(pid_t pid);

void deleteSemaphore(int semId);

int waitAndLockSemaphore(pid_t pid);

int unlockSemaphore(pid_t pid);


#endif //DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
