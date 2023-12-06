//
// Created by lelelele29 on 04/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H

int getSemaphore(int id);

void deleteSemaphore(int semId);

int waitSemaphore(int semId);

int waitAndLockSemaphore(int semId);

int unlockSemaphore(int semId);


#endif //DISALVOFRANZA_PROGETTOSO_SEMAPHOREUTILS_H
