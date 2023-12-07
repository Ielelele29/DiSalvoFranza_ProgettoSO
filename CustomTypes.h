#ifndef DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
#define DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H

#include "NodeUtils.h"

typedef enum {false, true} boolean;
typedef enum {MASTER_SIGNAL_SEMAPHORE} SemaphoreType;
typedef Node* Atom;

#define maxMessageLength 100
typedef struct {
    long messageType;
    char messageText[maxMessageLength];
} Message;

#endif //DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
