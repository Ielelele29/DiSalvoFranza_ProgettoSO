//
// Created by lelelele29 on 06/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_ATOMUTILS_H
#define DISALVOFRANZA_PROGETTOSO_ATOMUTILS_H

int createAtomMemory(int maxAtoms);

void deleteAtomMemory(int atomMemoryId);

int getAtomMemoryId(int maxAtoms);

int* getAtomMemory(int atomMemoryId);

int getFirstFreeAtomSlot(int atomMemoryId, int maxAtoms);

int getAtomSemaphore();

#endif //DISALVOFRANZA_PROGETTOSO_ATOMUTILS_H
