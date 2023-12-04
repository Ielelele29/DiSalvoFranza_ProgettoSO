#ifndef DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
#define DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H

typedef enum {false, true} boolean;

#define maxMessageLength 100
typedef struct {
    long messageType;
    char messageText[maxMessageLength];
} Message;

#endif //DISALVOFRANZA_PROGETTOSO_CUSTOMTYPES_H
