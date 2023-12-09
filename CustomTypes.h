#ifndef DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
#define DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H

typedef enum {false, true} boolean;
typedef enum {MASTER_SIGNAL_SEMAPHORE, STATISTICS_SEMAPHORE, INHIBITOR_SEMAPHORE} SemaphoreType;
typedef enum {ACTIVATION_AMOUNT, SPLIT_AMOUNT, ENERGY_PRODUCED, ENERGY_CONSUMED, CREATED_ATOMS, DEAD_ATOMS, DELAYED_ATOM_SPLIT, AVOIDED_EXPLOSIONS, AVOIDED_MELTDOWNS, ENERGY_AMOUNT} StatisticType;
typedef enum {TIMEOUT, EXPLODE, BLACKOUT, MELTDOWN} TerminationType;
typedef enum {STATISTICS_SHARED_MEMORY, INHIBITOR_SHARED_MEMORY} SharedMemoryType;

typedef struct Node {
    struct Node* previousNode;
    int value;
    struct Node* nextNode;
} Node;

typedef Node* Atom;

#define maxMessageLength 100
typedef struct {
    long messageType;
    char messageText[maxMessageLength];
} Message;

#endif //DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
