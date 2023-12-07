#ifndef DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
#define DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H

#include "NodeUtils.h"

typedef enum {false, true} boolean;
typedef enum {MASTER_SIGNAL_SEMAPHORE, STATISTICS_SEMAPHORE} SemaphoreType;
typedef enum {ACTIVATION_AMOUNT, SPLIT_AMOUNT, ENERGY_PRODUCED, ENERGY_CONSUMED, DELAYED_ATOM_SPLIT, AVOIDED_EXPLOSIONS, AVOIDED_MELTDOWNS, ENERGY_AMOUNT} StatisticType;
typedef enum {TIMEOUT, EXPLODE, BLACKOUT, MELTDOWN} TerminationType;
typedef Node* Atom;

#define maxMessageLength 100
typedef struct {
    long messageType;
    char messageText[maxMessageLength];
} Message;

#endif //DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
