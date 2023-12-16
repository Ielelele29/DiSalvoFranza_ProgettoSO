#ifndef DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H
#define DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H

#include "CustomTypes.h"

Message createMessage(long type, const char* string);

Message createEmptyMessage();

void sendMessage(int messageChannelId, Message message);

int getMessageId(pid_t pid);

void killMessageChannel(int messageChannelId);

#endif //DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H
