#ifndef DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
#define DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H

typedef enum boolean {false, true} boolean;
typedef enum SemaphoreType {STATISTICS_SEMAPHORE, CONFIG_SEMAPHORE} SemaphoreType;
typedef enum StatisticType {ACTIVATION_AMOUNT, SPLIT_AMOUNT, ENERGY_PRODUCED, ENERGY_CONSUMED, CREATED_ATOMS, DEAD_ATOMS, DELAYED_ATOM_SPLIT, AVOIDED_EXPLOSIONS, AVOIDED_MELTDOWNS, ABSORBED_ENERGY, ENERGY_AMOUNT} StatisticType;
typedef enum TerminationType {TIMEOUT, EXPLODE, BLACKOUT, MELTDOWN} TerminationType;
typedef enum SharedMemoryType {STATISTICS_SHARED_MEMORY, CONFIG_SHARED_MEMORY} SharedMemoryType;
typedef enum Config {ENERGY_DEMAND, ENERGY_EXPLODE_THRESHOLD, N_NUOVI_ATOMI, N_ATOMI_INIT, N_ATOM_MAX, MIN_N_ATOMICO, STEP, SIM_DURATION} Config;

typedef struct Node {
    struct Node* previousNode;
    int value;
    struct Node* nextNode;
} Node;

typedef Node* Atom;

#define MaxPid 4194304
#define MaxMessageLength 100
typedef struct Message {
    long messageType;
    char messageText[MaxMessageLength];
} Message;

#endif //DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
